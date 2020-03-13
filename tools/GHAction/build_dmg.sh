#!/usr/bin/env bash
set -ex

WORKSPACE=$(cd "$(dirname "$0")/../.." && pwd)
export WORKSPACE
cd "${WORKSPACE}"

if [ -n "$CERT_PASSWORD" ]
then
    security delete-keychain build.keychain || true
    security create-keychain -p 'Password123' build.keychain
    security default-keychain -s build.keychain
    security import "$WORKSPACE/tools/GHAction/Certificates.p12" -A -P "$CERT_PASSWORD"
    security unlock-keychain -p 'Password123' build.keychain
    security set-keychain-settings build.keychain
    security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k 'Password123' build.keychain

    mkdir -p "$HOME/Library/MobileDevice/Provisioning Profiles"
    for PLATFORM_DIR in iphone tvos
    do
        cp "$WORKSPACE/platform/$PLATFORM_DIR"/*.mobileprovision "$HOME/Library/MobileDevice/Provisioning Profiles/"
    done
fi

TCCDB="$HOME/Library/Application Support/com.apple.TCC/TCC.db"
if [ ! -f "$TCCDB" ]
then
	mkdir -p "$(dirname "$TCCDB")"
	sqlite3 "$TCCDB" '
	CREATE TABLE "access" ( service TEXT NOT NULL, client TEXT NOT NULL, client_type INTEGER NOT NULL, allowed INTEGER NOT NULL, prompt_count INTEGER NOT NULL, csreq BLOB, policy_id INTEGER, indirect_object_identifier_type INTEGER, indirect_object_identifier TEXT DEFAULT "UNUSED", indirect_object_code_identity BLOB, flags INTEGER, last_modified INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY (service, client, client_type, indirect_object_identifier), FOREIGN KEY (policy_id) REFERENCES policies(id) ON DELETE CASCADE ON UPDATE CASCADE);
	CREATE TABLE access_overrides ( service TEXT NOT NULL PRIMARY KEY);
	CREATE TABLE active_policy ( client TEXT NOT NULL, client_type INTEGER NOT NULL, policy_id INTEGER NOT NULL, PRIMARY KEY (client, client_type), FOREIGN KEY (policy_id) REFERENCES policies(id) ON DELETE CASCADE ON UPDATE CASCADE);
	CREATE TABLE admin (key TEXT PRIMARY KEY NOT NULL, value INTEGER NOT NULL);
	CREATE TABLE expired ( service TEXT NOT NULL, client TEXT NOT NULL, client_type INTEGER NOT NULL, csreq BLOB, last_modified INTEGER NOT NULL , expired_at INTEGER NOT NULL DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY (service, client, client_type));
	CREATE TABLE policies ( id INTEGER NOT NULL PRIMARY KEY, bundle_id TEXT NOT NULL, uuid TEXT NOT NULL, display TEXT NOT NULL, UNIQUE (bundle_id, uuid));
	INSERT INTO "admin" VALUES ("version", "15");'
fi
sqlite3 "$TCCDB" 'REPLACE into "access" VALUES ("kTCCServiceAppleEvents", "com.apple.Terminal", 0, 1, 1, NULL, NULL, NULL, "com.apple.finder", NULL, 0, 0), ("kTCCServiceAppleEvents", "/usr/bin/osascript", 1, 1, 1, NULL, NULL, NULL, "com.apple.finder", NULL, 0, 0);'

BUILD_NUMBER=${BUILD_NUMBER:-3575}
YEAR=${YEAR:-2020}

if ! bin/mac/build_dmg.sh -d -b "$YEAR.$BUILD_NUMBER" -e "${WORKSPACE}/Native/CoronaNative.tar.gz" "${WORKSPACE}" "${WORKSPACE}/docs"
then
    BUILD_FAILED=YES
    echo "BUILD FAILED"
fi

if [ -n "$CERT_PASSWORD" ]
then
    security default-keychain -s login.keychain
    security delete-keychain build.keychain &> /dev/null || true
fi

if [ "$BUILD_FAILED" = "YES" ]
then
    exit 1
fi
echo $?
