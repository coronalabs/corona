//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SQLiteStoredPreferences.h"
#include "Core\Rtt_Assert.h"
#include "Rtt_Preference.h"
#include "sqlite3.h"


namespace Interop { namespace Storage {

#pragma region Constructors/Destructors
SQLiteStoredPreferences::SQLiteStoredPreferences(const wchar_t* filePath)
:	fFilePath(filePath ? filePath : L""),
	fLastFileUpdateTime()
{
}

SQLiteStoredPreferences::~SQLiteStoredPreferences()
{
}

#pragma endregion


#pragma region Public Methods
const wchar_t* SQLiteStoredPreferences::GetFilePath() const
{
	return fFilePath.GetUTF16();
}

Rtt::Preference::ReadValueResult SQLiteStoredPreferences::Fetch(const char* keyName)
{
	// Determine if we need to re-sync with the database file.
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes{};
	BOOL wasFileFound = ::GetFileAttributesExW(fFilePath.GetUTF16(), ::GetFileExInfoStandard, &fileAttributes);
	if (!wasFileFound)
	{
		// File not found. Clear our cached preferences if applicable.
		fCachedPreferences.Clear();
		memset(&fLastFileUpdateTime, 0, sizeof(fLastFileUpdateTime));
	}
	else if (::CompareFileTime(&fLastFileUpdateTime, &fileAttributes.ftLastWriteTime) != 0)
	{
		// The database file was changed by another app. Re-sync our cached preferences with the newest changes.
		DatabaseConnection databaseConnection(*this);
		auto openResult = databaseConnection.OpenForReading();
		if (openResult.HasFailed())
		{
			return Rtt::Preference::ReadValueResult::FailedWith(openResult.GetUtf8Message());
		}
	}

	// Attempt to fetch the requested preference.
	auto preferencePointer = fCachedPreferences.GetByKey(keyName);
	if (!preferencePointer)
	{
		return Rtt::Preference::ReadValueResult::kPreferenceNotFound;
	}

	// The requested preference was found. Return its value.
	return Rtt::Preference::ReadValueResult::SucceededWith(preferencePointer->GetValue());
}

Rtt::OperationResult SQLiteStoredPreferences::UpdateWith(const Rtt::Preference& preference)
{
	Rtt::PreferenceCollection collection;
	collection.Add(preference);
	return UpdateWith(collection);
}

Rtt::OperationResult SQLiteStoredPreferences::UpdateWith(const Rtt::PreferenceCollection& preferences)
{
	// Do not continue if the collection is empty.
	if (preferences.GetCount() <= 0)
	{
		return Rtt::OperationResult::kSucceeded;
	}

	// Open the database for writing.
	DatabaseConnection databaseConnection(*this);
	auto openResult = databaseConnection.OpenForWriting();
	if (openResult.HasFailed())
	{
		return openResult;
	}

	// Attempt to insert/update all of the given preferences.
	return databaseConnection.UpdateWith(preferences);
}

Rtt::OperationResult SQLiteStoredPreferences::Delete(const char* keyName)
{
	// Validate.
	if (!keyName)
	{
		return Rtt::OperationResult::FailedWith("Preference key cannot be null.");
	}

	// Delete the preference.
	const char* keyNameArray[] = { keyName };
	return Delete(keyNameArray, 1);
}

Rtt::OperationResult SQLiteStoredPreferences::Delete(const char* keyNameArray[], int count)
{
	// Validate.
	if (!keyNameArray)
	{
		return Rtt::OperationResult::FailedWith("Key array cannot be null.");
	}
	if (count <= 0)
	{
		return Rtt::OperationResult::kSucceeded;
	}

	// Open the database for writing.
	DatabaseConnection databaseConnection(*this);
	auto openResult = databaseConnection.OpenForWriting();
	if (openResult.HasFailed())
	{
		return openResult;
	}

	// Attempt to delete all of the given preferences.
	return databaseConnection.Delete(keyNameArray, count);
}

#pragma endregion


#pragma region DatabaseConnection Methods
SQLiteStoredPreferences::DatabaseConnection::DatabaseConnection(SQLiteStoredPreferences& storedPreferences)
:	fStoredPreferences(storedPreferences),
	fDatabasePointer(nullptr)
{
}

SQLiteStoredPreferences::DatabaseConnection::~DatabaseConnection()
{
	Close();
}

bool SQLiteStoredPreferences::DatabaseConnection::IsOpen() const
{
	return (fDatabasePointer != nullptr);
}

Rtt::OperationResult SQLiteStoredPreferences::DatabaseConnection::OpenForReading()
{
	bool isForWriting = false;
	return Open(isForWriting);
}

Rtt::OperationResult SQLiteStoredPreferences::DatabaseConnection::OpenForWriting()
{
	bool isForWriting = true;
	return Open(isForWriting);
}

Rtt::OperationResult SQLiteStoredPreferences::DatabaseConnection::Open(bool isForWriting)
{
	std::string utf8ErrorMessage;

	// Do not continue if the database connection is already open.
	if (fDatabasePointer)
	{
		return Rtt::OperationResult::FailedWith("Database is already open.");
	}

	// Open a connection to the database.
	int openFlags = isForWriting ? (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE) : SQLITE_OPEN_READONLY;
	openFlags |= SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_PRIVATECACHE;
	for (int attemptCount = 1; attemptCount <= 2; attemptCount++)
	{
		// Attempt to open the database file.
		// Note: By default, this function will block for 2 seconds if the file is locked.
		int errorCode = sqlite3_open_v2(fStoredPreferences.fFilePath.GetUTF8(), &fDatabasePointer, openFlags, nullptr);
		if ((errorCode != SQLITE_OK) || !fDatabasePointer)
		{
			if (fDatabasePointer)
			{
				auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
				if (utf8MessagePointer)
				{
					utf8ErrorMessage = utf8MessagePointer;
				}
				sqlite3_close(fDatabasePointer);
				fDatabasePointer = nullptr;
			}
			break;
		}

		// Create the preferences table if not done already.
		// Note: If the file is corrupted or not a SQLite database file, then it'll be detected here.
		const char kSqlCreatePreferencesTableString[] =
				"CREATE TABLE IF NOT EXISTS preference (key TEXT PRIMARY KEY NOT NULL, value VARIANT)";
		errorCode = sqlite3_exec(fDatabasePointer, kSqlCreatePreferencesTableString, nullptr, nullptr, nullptr);
		if (errorCode != SQLITE_OK)
		{
			auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
			if (utf8MessagePointer)
			{
				utf8ErrorMessage = utf8MessagePointer;
			}
			sqlite3_close(fDatabasePointer);
			fDatabasePointer = nullptr;
			if ((SQLITE_NOTADB == errorCode) || (SQLITE_CORRUPT == errorCode))
			{
				// The file is corrupted or not a SQLite database file. Delete the file and try again.
				BOOL wasDeleted = ::DeleteFileW(fStoredPreferences.fFilePath.GetUTF16());
				if (wasDeleted)
				{
					fStoredPreferences.fCachedPreferences.Clear();
					memset(&fStoredPreferences.fLastFileUpdateTime, 0, sizeof(fStoredPreferences.fLastFileUpdateTime));
					continue;
				}

				// Unable to delete the invalid file. It's likely locked. Give up.
				break;
			}
		}

		// We've successfully opened a connection to the database file.
		break;
	}

	// Do not continue if we've failed to open the database above.
	if (!fDatabasePointer)
	{
		if (utf8ErrorMessage.empty())
		{
			utf8ErrorMessage = "Failed to open database file.";
		}
		return Rtt::OperationResult::FailedWith(utf8ErrorMessage.c_str());
	}

	// Check if the database file has been updated by an external app by checking the file's timestamp.
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes{};
	BOOL wasFileFound = ::GetFileAttributesExW(
			fStoredPreferences.fFilePath.GetUTF16(), ::GetFileExInfoStandard, &fileAttributes);
	if (::CompareFileTime(&fStoredPreferences.fLastFileUpdateTime, &fileAttributes.ftLastWriteTime) != 0)
	{
		// The database file was altered externally. Our cached preferences might be out of sync with the file.
		// Remove our last fetched preferences from memory.
		fStoredPreferences.fCachedPreferences.Clear();

		// Fetch all preferences from the database.
		const char kSqlSelectPreferencesString[] = "SELECT key, value FROM preference";
		sqlite3_stmt* sqlStatementPointer = nullptr;
		sqlite3_prepare_v2(fDatabasePointer, kSqlSelectPreferencesString, -1, &sqlStatementPointer, nullptr);
		if (sqlStatementPointer && (sqlite3_column_count(sqlStatementPointer) >= 2))
		{
			while (sqlite3_step(sqlStatementPointer) == SQLITE_ROW)
			{
				// Fetch the preference's unique name.
				const char* utf8KeyName = (const char*)sqlite3_column_text(sqlStatementPointer, 0);
				if (!utf8KeyName)
				{
					continue;
				}

				// Fetch the preference's value. (Stored as a variant within the database.)
				auto sqlVariantValuePointer = sqlite3_column_value(sqlStatementPointer, 1);
				if (!sqlVariantValuePointer)
				{
					continue;
				}
				Rtt::Preference preference(utf8KeyName);
				switch (sqlite3_value_type(sqlVariantValuePointer))
				{
					case SQLITE_INTEGER:
					case SQLITE_FLOAT:
					case SQLITE_TEXT:
					{
						preference.SetValue(
								Rtt::PreferenceValue((const char*)sqlite3_value_text(sqlVariantValuePointer)));
						break;
					}
					case SQLITE_BLOB:
					{
						auto byteBufferPointer = sqlite3_value_blob(sqlVariantValuePointer);
						auto byteBufferLength = sqlite3_value_bytes(sqlVariantValuePointer);
						if (byteBufferPointer && (byteBufferLength > 0))
						{
							auto utf8StringPointer = Rtt_MakeSharedConstStdStringPtr(
									(const char*)byteBufferPointer, (size_t)byteBufferLength);
							preference.SetValue(Rtt::PreferenceValue(utf8StringPointer));
						}
						else
						{
							preference.SetValue(Rtt::PreferenceValue(""));
						}
						break;
					}
					case SQLITE_NULL:
					{
						preference.SetValue(Rtt::PreferenceValue(""));
						break;
					}
					default:
					{
						continue;
					}
				}

				// Add the fetched preference key/value pair to our collection for fast access.
				fStoredPreferences.fCachedPreferences.Add(preference);
			}
		}
		if (sqlStatementPointer)
		{
			sqlite3_finalize(sqlStatementPointer);
			sqlStatementPointer = nullptr;
		}
	}

	// Database connection was opened successfuly.
	return Rtt::OperationResult::kSucceeded;
}

void SQLiteStoredPreferences::DatabaseConnection::Close()
{
	// Do not continue if already closed.
	if (!fDatabasePointer)
	{
		return;
	}

	// Close the database connection.
	// This will also finalize any pending writes to file.
	sqlite3_close(fDatabasePointer);
	fDatabasePointer = nullptr;

	// Store the file's current updated/modified timestamp.
	// We use this to determine if the database was updated by another app for re-syncing purposes.
	WIN32_FILE_ATTRIBUTE_DATA fileAttributes{};
	BOOL wasFileFound = ::GetFileAttributesExW(
			fStoredPreferences.fFilePath.GetUTF16(), ::GetFileExInfoStandard, &fileAttributes);
	if (wasFileFound)
	{
		fStoredPreferences.fLastFileUpdateTime = fileAttributes.ftLastWriteTime;
	}
}

Rtt::OperationResult SQLiteStoredPreferences::DatabaseConnection::UpdateWith(
	const Rtt::PreferenceCollection& preferences)
{
	std::string utf8ErrorMessage;
	int sqlErrorCode;

	// Do not continue if the database connection is not open.
	if (!fDatabasePointer)
	{
		return Rtt::OperationResult::FailedWith("Database connection is not open.");
	}

	// Do not continue if the given preference collection is empty.
	if (preferences.GetCount() <= 0)
	{
		return Rtt::OperationResult::kSucceeded;
	}

	// Start an SQL transaction.
	// This is an optimization. It allows us to insert all preferences to file in one shot.
	sqlErrorCode = sqlite3_exec(fDatabasePointer, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
	if (sqlErrorCode != SQLITE_OK)
	{
		auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
		if (utf8MessagePointer)
		{
			utf8ErrorMessage = utf8MessagePointer;
		}
		return Rtt::OperationResult::FailedWith(utf8ErrorMessage.c_str());
	}

	// Insert all of the given preferences via parameterized queries.
	// Abort if at least one preference fails to be inserted.
	bool wasSuccessful = false;
	const char kSqlInsertPreferencesString[] = "INSERT OR REPLACE INTO preference (key, value) VALUES (?, ?)";
	sqlite3_stmt* sqlStatementPointer = nullptr;
	sqlErrorCode = sqlite3_prepare_v2(fDatabasePointer, kSqlInsertPreferencesString, -1, &sqlStatementPointer, nullptr);
	if (sqlErrorCode != SQLITE_OK)
	{
		auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
		if (utf8MessagePointer)
		{
			utf8ErrorMessage = utf8MessagePointer;
		}
	}
	if (sqlStatementPointer)
	{
		wasSuccessful = true;
		for (int index = preferences.GetCount() - 1; index >= 0; index--)
		{
			// Fetch the next preference to write to the database.
			auto preferencePointer = preferences.GetByIndex(index);
			if (!preferencePointer)
			{
				continue;
			}

			// Convert the preference value to a type supported by SQLite.
			Rtt::PreferenceValue::TypeSet supportedTypes;
			supportedTypes.Add(Rtt::PreferenceValue::kTypeSignedInt32);
			supportedTypes.Add(Rtt::PreferenceValue::kTypeSignedInt64);
			supportedTypes.Add(Rtt::PreferenceValue::kTypeFloatDouble);
			supportedTypes.Add(Rtt::PreferenceValue::kTypeString);
			auto valueConversionResult = preferencePointer->GetValue().ToClosestValueTypeIn(supportedTypes);
			if (valueConversionResult.HasFailed())
			{
				auto utf8ErrorMessagePointer = valueConversionResult.GetUtf8Message();;
				if (utf8ErrorMessagePointer)
				{
					utf8ErrorMessage = utf8ErrorMessagePointer;
				}
				wasSuccessful = false;
				break;
			}

			// Bind the preference's key name.
			auto utf8KeyNamePointer = preferencePointer->GetKeyNameAsSharedPointer();
			sqlite3_bind_text(
					sqlStatementPointer, 1, utf8KeyNamePointer->c_str(),
					utf8KeyNamePointer->length() * sizeof(char), nullptr);

			// Bind the preference's value.
			auto preferenceValue = valueConversionResult.GetValue();
			switch (preferenceValue.GetType())
			{
				case Rtt::PreferenceValue::kTypeSignedInt32:
				{
					// Insert the preference value as a 32-bit integer.
					sqlite3_bind_int(sqlStatementPointer, 2, preferenceValue.ToSignedInt32().GetValue());
					break;
				}
				case Rtt::PreferenceValue::kTypeSignedInt64:
				{
					// Insert the preference value as a 64-bit integer.
					sqlite3_bind_int64(sqlStatementPointer, 2, preferenceValue.ToSignedInt64().GetValue());
					break;
				}
				case Rtt::PreferenceValue::kTypeFloatDouble:
				{
					// Insert the preference value as a double precision float.
					sqlite3_bind_double(sqlStatementPointer, 2, preferenceValue.ToFloatDouble().GetValue());
					break;
				}
				default:
				{
					// Insert the preference value as string.
					auto stringPointer = preferenceValue.ToString().GetValue();
					if (stringPointer.NotNull())
					{
						auto nullCharacterIndex = stringPointer->find('\0');
						if ((nullCharacterIndex != std::string::npos) &&
						    ((nullCharacterIndex + 1) < stringPointer->length()))
						{
							// The string contains at least 1 embedded null character.
							// Insert it as a blob so that the string won't get truncated when retrieving it later.
							sqlite3_bind_blob(
									sqlStatementPointer, 2, stringPointer->c_str(), stringPointer->length(), nullptr);
						}
						else
						{
							// Insert the value as a null terminated string.
							// This is preferred since it'll be human readable in a database viewer.
							sqlite3_bind_text(
									sqlStatementPointer, 2, stringPointer->c_str(), stringPointer->length(), nullptr);
						}
					}
					else
					{
						// Insert a null value for null string pointers.
						sqlite3_bind_null(sqlStatementPointer, 2);
					}
					break;
				}
			}

			// Execute the SQL statement.
			sqlErrorCode = sqlite3_step(sqlStatementPointer);
			if ((sqlErrorCode != SQLITE_DONE) && (sqlErrorCode != SQLITE_OK))
			{
				auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
				if (utf8MessagePointer)
				{
					utf8ErrorMessage = utf8MessagePointer;
				}
				wasSuccessful = false;
			}

			// Clear the SQL bindings for the next preference to be inserted.
			sqlite3_clear_bindings(sqlStatementPointer);
			sqlite3_reset(sqlStatementPointer);

			// Abort if we've failed to insert the preference.
			if (!wasSuccessful)
			{
				break;
			}
		}
	}
	if (sqlStatementPointer)
	{
		sqlite3_finalize(sqlStatementPointer);
		sqlStatementPointer = nullptr;
	}

	// End the SQL transaction. This commits the above insertions into the database, if successful.
	const char* kSqlEndStatementString = wasSuccessful ? "END TRANSACTION" : "ROLLBACK";
	sqlErrorCode = sqlite3_exec(fDatabasePointer, kSqlEndStatementString, nullptr, nullptr, nullptr);
	if (sqlErrorCode != SQLITE_OK)
	{
		auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
		if (utf8MessagePointer)
		{
			utf8ErrorMessage = utf8MessagePointer;
		}
		wasSuccessful = false;
	}
	if (!wasSuccessful)
	{
		if (utf8ErrorMessage.empty())
		{
			utf8ErrorMessage = "Unknown database error.";
		}
		return Rtt::OperationResult::FailedWith(utf8ErrorMessage.c_str());
	}

	// Update the cached preferences with all of the preferences we've successfully inserted into the database.
	for (int index = preferences.GetCount() - 1; index >= 0; index--)
	{
		auto newPreferencePointer = preferences.GetByIndex(index);
		if (newPreferencePointer)
		{
			auto cachedPreferencePointer =
					fStoredPreferences.fCachedPreferences.GetByKey(newPreferencePointer->GetKeyName());
			if (cachedPreferencePointer)
			{
				cachedPreferencePointer->SetValue(newPreferencePointer->GetValue());
			}
			else
			{
				fStoredPreferences.fCachedPreferences.Add(*newPreferencePointer);
			}
		}
	}
	return Rtt::OperationResult::kSucceeded;
}

Rtt::OperationResult SQLiteStoredPreferences::DatabaseConnection::Delete(const char* keyNameArray[], int count)
{
	std::string utf8ErrorMessage;
	int sqlErrorCode;

	// Validate.
	if (!keyNameArray)
	{
		Rtt_ASSERT(0);
		return Rtt::OperationResult::FailedWith("Preference key array is null.");
	}

	// Do not continue if the database connection is not open.
	if (!fDatabasePointer)
	{
		return Rtt::OperationResult::FailedWith("Database connection is not open.");
	}

	// Do not continue if the given array is empty.
	if (count <= 0)
	{
		return Rtt::OperationResult::kSucceeded;
	}

	// Start an SQL transaction.
	// This is an optimization. It allows us to insert all preferences to file in one shot.
	sqlErrorCode = sqlite3_exec(fDatabasePointer, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
	if (sqlErrorCode != SQLITE_OK)
	{
		auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
		if (utf8MessagePointer)
		{
			utf8ErrorMessage = utf8MessagePointer;
		}
		return Rtt::OperationResult::FailedWith(utf8ErrorMessage.c_str());
	}

	// Delete all of the given preferences via parameterized queries.
	// Abort if at least one preference fails to be deleted.
	bool wasSuccessful = false;
	const char kSqlDeletePreferenceString[] = "DELETE FROM preference WHERE key = ?";
	sqlite3_stmt* sqlStatementPointer = nullptr;
	sqlErrorCode = sqlite3_prepare_v2(fDatabasePointer, kSqlDeletePreferenceString, -1, &sqlStatementPointer, nullptr);
	if (sqlErrorCode != SQLITE_OK)
	{
		auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
		if (utf8MessagePointer)
		{
			utf8ErrorMessage = utf8MessagePointer;
		}
	}
	if (sqlStatementPointer)
	{
		wasSuccessful = true;
		for (int index = 0; index < count; index++)
		{
			// Fetch the next preference key.
			const char* utf8KeyName = nullptr;
			try
			{
				utf8KeyName = keyNameArray[index];
			}
			catch (...)
			{
				Rtt_ASSERT(0);
				wasSuccessful = false;
				utf8ErrorMessage = "Exception occurred while accessing preference key array.";
				break;
			}
			if (!utf8KeyName)
			{
				continue;
			}

			// Bind the preference's key name.
			sqlite3_bind_text(sqlStatementPointer, 1, utf8KeyName, strlen(utf8KeyName) * sizeof(char), nullptr);

			// Execute the SQL statement.
			sqlErrorCode = sqlite3_step(sqlStatementPointer);
			if ((sqlErrorCode != SQLITE_DONE) && (sqlErrorCode != SQLITE_OK))
			{
				auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
				if (utf8MessagePointer)
				{
					utf8ErrorMessage = utf8MessagePointer;
				}
				wasSuccessful = false;
			}

			// Clear the SQL bindings for the next preference to be deleted.
			sqlite3_clear_bindings(sqlStatementPointer);
			sqlite3_reset(sqlStatementPointer);

			// Abort if we've failed to delete the above preference.
			if (!wasSuccessful)
			{
				break;
			}
		}
	}
	if (sqlStatementPointer)
	{
		sqlite3_finalize(sqlStatementPointer);
		sqlStatementPointer = nullptr;
	}

	// End the SQL transaction. This commits the above deletions to the database.
	const char* kSqlEndStatementString = wasSuccessful ? "END TRANSACTION" : "ROLLBACK";
	sqlErrorCode = sqlite3_exec(fDatabasePointer, kSqlEndStatementString, nullptr, nullptr, nullptr);
	if (sqlErrorCode != SQLITE_OK)
	{
		auto utf8MessagePointer = sqlite3_errmsg(fDatabasePointer);
		if (utf8MessagePointer)
		{
			utf8ErrorMessage = utf8MessagePointer;
		}
		wasSuccessful = false;
	}
	if (!wasSuccessful)
	{
		if (utf8ErrorMessage.empty())
		{
			utf8ErrorMessage = "Unknown database error.";
		}
		return Rtt::OperationResult::FailedWith(utf8ErrorMessage.c_str());
	}

	// Remove the given preferences from the cache.
	for (int index = 0; index < count; index++)
	{
		try
		{
			fStoredPreferences.fCachedPreferences.RemoveByKey(keyNameArray[index]);
		}
		catch (...) {}
	}
	return Rtt::OperationResult::kSucceeded;
}

#pragma endregion

} }	// namespace Interop::Storage
