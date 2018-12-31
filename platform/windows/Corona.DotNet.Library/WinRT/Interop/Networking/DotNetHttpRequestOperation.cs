using System;


namespace CoronaLabs.Corona.WinRT.Interop.Networking
{
	/// <summary>
	///  <para>Sends an HTTP request to a server and receives an HTTP response.</para>
	///  <para>This is implemented via a .NET <see cref="System.Net.HttpWebRequest">HttpWebRequest</see> class.</para>
	/// </summary>
	public class DotNetHttpRequestOperation : IHttpRequestOperation
	{
		#region Private Member Variables
		/// <summary>Stores the current HTTP request settings made available publicly via the "Settings" property.</summary>
		private HttpRequestSettings fPublicSettings;

		/// <summary>
		///  <para>Clone of member variable "fPublicSettings" when the Execute() method gets called.</para>
		///  <para>This is what the HTTP operation uses so that changes to public settings won't cause race conditions.</para>
		/// </summary>
		private HttpRequestSettings fClonedSettings;

		/// <summary>
		///  <para>Implements the HTTP request operation.</para>
		///  <para>Set to null if the HTTP request operation has not started yet or has finished executing.</para>
		/// </summary>
		private System.Net.HttpWebRequest fHttpWebRequest;

		#endregion


		#region Events
		/// <summary>
		///  <para>Raised just before connecting to the specified server to send the request to.</para>
		///  <para>Provides the number of bytes to be sent to the server.</para>
		///  <para>This event is raised on the same thread that called this object's Execute() method.</para>
		/// </summary>
		public event EventHandler<HttpProgressEventArgs> SendingRequest;

		/// <summary>
		///  <para>Raised periodically as this operation object sends HTTP request bytes to the server in batches.</para>
		///  <para>
		///   Will not be raised if the HTTP request packet size is small and is sent in one shot, in which case,
		///   the AsyncSentRequest event will be immediately raised after the SendingRequest event.
		///  </para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		public event EventHandler<HttpProgressEventArgs> AsyncSendProgressChanged;

		/// <summary>
		///  <para>Raised when this object has finished sending the full HTTP request to the server.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		public event EventHandler<HttpProgressEventArgs> AsyncSentRequest;

		/// <summary>
		///  <para>Raised when the beginning of the HTTP response (ie: its headers) has been received from the server.</para>
		///  <para>Note that this may provide the number of bytes expected to be received from the server.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		public event EventHandler<HttpResponseEventArgs> AsyncReceivingResponse;

		/// <summary>
		///  <para>Raised periodically as HTTP response bytes are being received from the server.</para>
		///  <para>
		///   Will not be raised if the full HTTP response has been received in one shot, in which case,
		///   the AsyncReceivedResponse event will be raised immediately after the AsyncReceivingResponse event.
		///  </para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		public event EventHandler<HttpProgressEventArgs> AsyncReceiveProgressChanged;

		/// <summary>
		///  <para>Raised when the full HTTP response has been received from the server.</para>
		///  <para>This operation will be flagged as completed at this point and is free to execute another HTTP request.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		public event EventHandler<HttpResponseEventArgs> AsyncReceivedResponse;

		/// <summary>
		///  <para>Raised when a connection error or response timeout occurs.</para>
		///  <para>Note that this event does not get raised when an HTTP error response has been received.</para>
		///  <para>Warning! This event is raised on a separate thread and not on the main UI thread.</para>
		/// </summary>
		public event EventHandler<CoronaLabs.WinRT.MessageEventArgs> AsyncErrorOccurred;

		/// <summary>
		///  <para>Raised when the Abort() method gets called.</para>
		///  <para>This event is raised on the same thread that called this object's Abort() method.</para>
		/// </summary>
		public event EventHandler<CoronaLabs.WinRT.EmptyEventArgs> Aborted;

		#endregion


		#region Constructors
		/// <summary>Creates a new object used to invoke a single HTTP request operation.</summary>
		public DotNetHttpRequestOperation()
		{
			fPublicSettings = new HttpRequestSettings();
			fClonedSettings = null;
			fHttpWebRequest = null;
		}

		#endregion


		#region Public Methods/Properties
		/// <summary>
		///  Gets modifiable settings used to provide the HTTP request data to be sent and how to handle the received response.
		/// </summary>
		/// <value>
		///  Settings providing the HTTP request information to be sent and configuration on how to handle the response.
		/// </value>
		public HttpRequestSettings Settings
		{
			get { return fPublicSettings; }
		}

		/// <summary>Determines if this operation is in the middle of handling an HTTP request/response.</summary>
		/// <value>
		///  <para>
		///   Returns true if this operation object's <see cref="Execute()"/> method has been called and is in the middle
		///   of sending an HTTP request or handling an HTTP response.
		///  </para>
		///  <para>
		///   Returns false if the <see cref="Execute()"/> method has not been called yet or the last executed HTTP request has
		///   been completed, aborted, or erroed out.
		///  </para>
		/// </value>
		public bool IsExecuting
		{
			get { return (fHttpWebRequest != null); }
		}

		/// <summary>
		///  <para>Sends an HTTP request and receives an HTTP response based on the <see cref="Settings"/> property.</para>
		///  <para>
		///   A call to this method will be ignored if this operation object is already in the middle of executing
		///   an HTTP request operation.
		///  </para>
		/// </summary>
		/// <returns>
		///  <para>Returns a success result if the HTTP request operation has been started.</para>
		///  <para>
		///   Returns a failure result if the <see cref="Settings"/> property contains invalid information or if
		///   this operation object is already in the middle of executing an HTTP request. In this case, the returned
		///   result object will typically provide an error message detailing why it failed.
		///  </para>
		/// </returns>
		public CoronaLabs.WinRT.IOperationResult Execute()
		{
			// Do not continue if currently executing an operation.
			if (this.IsExecuting)
			{
				return CoronaLabs.WinRT.OperationResult.FailedWith(
							"Corona's HttpRequestOperation class can only send 1 request at a time.");
			}

			// Clone this operation's settings.
			var settings = new HttpRequestSettings();
			settings.CopyFrom(fPublicSettings);

			// Create and set up the HTTP request handling object.
			System.Net.HttpWebRequest httpWebRequest = null;
			try
			{
				// Create the HTTP request object with the given URL.
				if (settings.Uri == null)
				{
					return CoronaLabs.WinRT.OperationResult.FailedWith("URL not provided.");
				}
				httpWebRequest = System.Net.HttpWebRequest.CreateHttp(settings.Uri);

				// Set the HTTP request method.
				if (settings.MethodName != null)
				{
					settings.MethodName = settings.MethodName.Trim();
				}
				if (string.IsNullOrEmpty(settings.MethodName))
				{
					settings.MethodName = "GET";
				}
				httpWebRequest.Method = settings.MethodName;

				// Enable/disable auto-redirects.
				httpWebRequest.AllowAutoRedirect = settings.AutoRedirectEnabled;
			}
			catch (Exception ex)
			{
				return CoronaLabs.WinRT.OperationResult.FailedWith(ex.Message);
			}
			
			// Calculate the request's content length.
			// Note: The HTTP protocol does not support a GET or HEAD request to have content.
			var upperCaseMethodName = settings.MethodName.ToUpper();
			if ((upperCaseMethodName != "GET") && (upperCaseMethodName != "HEAD"))
			{
				if (settings.UploadFileEnabled)
				{
					// We're uploading the contents of a file.
					try
					{
						if (System.IO.File.Exists(settings.UploadFilePath) == false)
						{
							return CoronaLabs.WinRT.OperationResult.FailedWith("The file to upload could not be found.");
						}
						var fileInfo = new System.IO.FileInfo(settings.UploadFilePath);
						httpWebRequest.ContentLength = fileInfo.Length;
					}
					catch (Exception ex)
					{
						return CoronaLabs.WinRT.OperationResult.FailedWith(ex.Message);
					}
				}
				else
				{
					// We're uploading the byte buffer copied to this object's settings.
					if (settings.BytesToUpload == null)
					{
						settings.BytesToUpload = CoronaLabs.WinRT.ImmutableByteBuffer.Empty;
					}
					httpWebRequest.ContentLength = settings.BytesToUpload.Count;
				}
			}

			// Copy the HTTP header name/value pairs.
			foreach (var headerPair in settings.Headers)
			{
				// Get the next header name in lower-case form.
				if (headerPair.Key == null)
				{
					continue;
				}
				var trimmedHeaderName = headerPair.Key.Trim();
				if (string.IsNullOrEmpty(trimmedHeaderName))
				{
					continue;
				}
				var lowerCaseHeaderName = headerPair.Key.Trim().ToLower();

				// Apply the header setting to the HTTP request object.
				if (lowerCaseHeaderName == "accept")
				{
					httpWebRequest.Accept = headerPair.Value;
				}
				else if (lowerCaseHeaderName == "content-length")
				{
					// Ignore the "Content-Length" header entry. That was applied separately up above.
				}
				else if (lowerCaseHeaderName == "content-type")
				{
					httpWebRequest.ContentType = headerPair.Value;
				}
				else if (lowerCaseHeaderName == "user-agent")
				{
					httpWebRequest.UserAgent = headerPair.Value;
				}
				else
				{
					httpWebRequest.Headers[headerPair.Key] = headerPair.Value;
				}
			}

			// Notify the system that we're about to send the HTTP request.
			// This also provides the number of bytes to be uploaded.
			if (this.SendingRequest != null)
			{
				long bytesUploaded = 0;
				long totalBytesToUpload = (httpWebRequest.ContentLength >= 0) ? httpWebRequest.ContentLength : 0;
				this.SendingRequest.Invoke(this, new HttpProgressEventArgs(bytesUploaded, totalBytesToUpload));
			}

			// Send the HTTP request on another thread.
			try
			{
				fHttpWebRequest = httpWebRequest;
				fClonedSettings = settings;

				var thread = new System.Threading.Thread(OnAsyncConnect);
				thread.Start();
			}
			catch (Exception ex)
			{
				OnAbort();
				return CoronaLabs.WinRT.OperationResult.FailedWith(ex.Message);
			}

			// We've successfully started the HTTP request operation. It will work asynchronously from here.
			return CoronaLabs.WinRT.OperationResult.Succeeded;
		}

		/// <summary>
		///  <para>Aborts the currently executing HTTP request operation, if running.</para>
		///  <para>Will raise an Aborted event if this operation object is currently executing.</para>
		///  <para>
		///   Once aborted, the <see cref="IsExecuting"/> property will be set false and no more events will be raised
		///   until the next call to the <see cref="Execute()"/> method.
		///  </para>
		/// </summary>
		public void Abort()
		{
			// Do not continue if there is nothing to abort.
			if (this.IsExecuting == false)
			{
				return;
			}

			// Abort the HTTP request.
			OnAbort();

			// Notify the system that the HTTP operation was aborted.
			var abortedEventHandler = this.Aborted;
			if (abortedEventHandler != null)
			{
				abortedEventHandler.Invoke(this, CoronaLabs.WinRT.EmptyEventArgs.Instance);
			}
		}

		#endregion


		#region Private Methods
		/// <summary>Called on the network thread when attempting to connect to the host/server.</summary>
		private void OnAsyncConnect()
		{
			// Fetch the HTTP request implementation object and settings.
			var httpWebRequest = fHttpWebRequest;
			var settings = fClonedSettings;
			if ((httpWebRequest == null) || (settings == null))
			{
				return;
			}

			// Open a network connection to the server.
			// Note: This can block for about a minute if the server is unreachable.
			try
			{
				if (httpWebRequest.ContentLength >= 0)
				{
					httpWebRequest.BeginGetRequestStream(new AsyncCallback(OnAsyncGetRequestStream), httpWebRequest);
				}
				else
				{
					httpWebRequest.BeginGetResponse(OnAsyncGetResponse, httpWebRequest);
				}
			}
			catch (Exception ex)
			{
				AbortWithError(ex.Message);
			}
		}

		/// <summary>
		///  Called on the network thread when its time to sent the HTTP request's body/content to the host/server.
		/// </summary>
		/// <param name="result">Provides the HttpWebRequest object that invoked this method.</param>
		private void OnAsyncGetRequestStream(IAsyncResult result)
		{
			// Fetch the HTTP request implementation object and settings.
			var httpWebRequest = fHttpWebRequest;
			var settings = fClonedSettings;
			if ((httpWebRequest == null) || (settings == null))
			{
				return;
			}

			// Do not continue if this operation has been aborted.
			if (object.ReferenceEquals(httpWebRequest, result.AsyncState) == false)
			{
				return;
			}

			// Acquire a stream used to output the HTTP request's bytes to.
			System.IO.Stream outputStream = null;
			try
			{
				outputStream = httpWebRequest.EndGetRequestStream(result);
			}
			catch (Exception ex)
			{
				AbortWithError(ex.Message);
				return;
			}
			if (outputStream == null)
			{
				AbortWithError("Failed to acquire HTTP request stream.");
				return;
			}

			// Do not continue if this operation has been aborted.
			if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
			{
				return;
			}

			// Send the HTTP request body, if available.
			if (httpWebRequest.ContentLength > 0)
			{
				System.IO.Stream inputStream = null;
				try
				{
					// If uploading a file, then open a stream to it.
					if (settings.UploadFileEnabled)
					{
						inputStream = System.IO.File.OpenRead(settings.UploadFilePath);
					}

					// Stream out the HTTP request body in batches of 2 KB.
					var byteBuffer = new byte[2048];
					for (long bytesSent = 0; bytesSent < httpWebRequest.ContentLength; )
					{
						// Notify the system how many bytes have been uploaded so far.
						// Note: We do not raise this event on the first or last pass of this loop because those are
						//       handled by the "SendingRequest" and "AsyncSentRequest" events respectively.
						if (bytesSent > 0)
						{
							var asyncSendProgressChangedEventHandler = this.AsyncSendProgressChanged;
							if (asyncSendProgressChangedEventHandler != null)
							{
								asyncSendProgressChangedEventHandler.Invoke(
										this, new HttpProgressEventArgs(bytesSent, httpWebRequest.ContentLength));
							}
						}

						// Do not continue if this operation has been aborted.
						if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
						{
							return;
						}

						// Fetch the next batch of request content bytes.
						int bytesRead = 0;
						if (inputStream != null)
						{
							// Copy the next batch of bytes from the file stream.
							bytesRead = inputStream.Read(byteBuffer, 0, byteBuffer.Length);
						}
						else if (settings.BytesToUpload != null)
						{
							// Copy the next batch of bytes from the byte buffer.
							int bytesRemaining = settings.BytesToUpload.Count - (int)bytesSent;
							if (bytesRemaining < 0)
							{
								bytesRemaining = 0;
							}
							int bytesToRead = (bytesRemaining < byteBuffer.Length) ? bytesRemaining : byteBuffer.Length;
							for (int index = 0; index < bytesToRead; index++)
							{
								byteBuffer[index] = settings.BytesToUpload.GetByIndex(index + (int)bytesSent);
							}
							bytesRead = bytesToRead;
						}

						// Send the bytes to the server.
						if (bytesRead > 0)
						{
							outputStream.Write(byteBuffer, 0, bytesRead);
							bytesSent += (long)bytesRead;
						}
					}
				}
				catch (Exception ex)
				{
					// We've most likely failed write to the stream or failed to open the file to upload.
					try { outputStream.Close(); }
					catch (Exception) { }
					AbortWithError(ex.Message);
					return;
				}
				finally
				{
					// If we were uploading a file, then close it.
					if (inputStream != null)
					{
						try
						{
							inputStream.Close();
							inputStream.Dispose();
						}
						catch (Exception) { }
					}
				}
			}

			// We're done uploading the HTTP request. Close its stream.
			try
			{
				outputStream.Close();
			}
			catch (Exception ex)
			{
				AbortWithError(ex.Message);
				return;
			}

			// Notify the system that we've finished sending the HTTP request.
			var asyncSentRequestEventHandler = this.AsyncSentRequest;
			if (asyncSentRequestEventHandler != null)
			{
				long bytesSent = (httpWebRequest.ContentLength >= 0) ? httpWebRequest.ContentLength : 0;
				asyncSentRequestEventHandler.Invoke(this, new HttpProgressEventArgs(bytesSent, bytesSent));
			}

			// Do not continue if the handler for the above event has aborted this operation.
			if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
			{
				return;
			}

			// Wait for the HTTP response.
			try
			{
				httpWebRequest.BeginGetResponse(OnAsyncGetResponse, httpWebRequest);
			}
			catch (Exception ex)
			{
				AbortWithError(ex.Message);
			}
		}

		/// <summary>
		///  Called on the network thread when it is time to receive an HTTP response from the host/server.
		/// </summary>
		/// <param name="result">Provides the HttpWebRequest object that invoked this method.</param>
		private void OnAsyncGetResponse(IAsyncResult result)
		{
			// Fetch the HTTP request implementation object and settings.
			var httpWebRequest = fHttpWebRequest;
			var settings = fClonedSettings;
			if ((httpWebRequest == null) || (settings == null))
			{
				return;
			}

			// Do not continue if this operation has been aborted.
			if (object.ReferenceEquals(httpWebRequest, result.AsyncState) == false)
			{
				return;
			}

			// Fetch the HTTP response data.
			System.Net.HttpWebResponse httpWebResponse = null;
			System.IO.Stream inputStream = null;
			System.IO.Stream outputStream = null;
			try
			{
				// Fetch the HTTP response.
				try
				{
					httpWebResponse = httpWebRequest.EndGetResponse(result) as System.Net.HttpWebResponse;
				}
				catch (System.Net.WebException ex)
				{
					// Check if an exception was thrown because we've received an HTTP error response.
					// This can happen if the response's status code is >= 400.
					httpWebResponse = ex.Response as System.Net.HttpWebResponse;
					
					// If we did not get a response, then assume its a network error and abort.
					if (httpWebResponse == null)
					{
						throw ex;
					}

#if WINDOWS_PHONE
					// If the Internet connection is down, then we'll get a fake 404 response.
					// If this is the case, then error out immediately. (The HTTP response is not real. So, don't provide it.)
					if (Microsoft.Phone.Net.NetworkInformation.DeviceNetworkInformation.IsNetworkAvailable == false)
					{
						throw new Exception("Network connection error.");
					}
#endif
				}

				// If the HTTP request had no content, then the OnAsyncGetRequestStream() never got called.
				// Notify the system that we've finished sending the HTTP request, which happened in EndGetResponse() above.
				if (httpWebRequest.ContentLength < 0)
				{
					var asyncSentRequestEventHandler = this.AsyncSentRequest;
					if (asyncSentRequestEventHandler != null)
					{
						asyncSentRequestEventHandler.Invoke(this, new HttpProgressEventArgs(0, 0));
					}
				}

				// Do not continue if the handler for the above event has aborted this operation.
				if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
				{
					return;
				}

				// Fetch the estimated number of bytes in the HTTP response's body.
				// If the value is -1 or 4294967295 (the unsigned version of -1), then the amount of bytes is unknown.
				inputStream = httpWebResponse.GetResponseStream();
				long responseContentLength = httpWebResponse.ContentLength;
				if ((responseContentLength < 0) || (responseContentLength == (long)UInt32.MaxValue))
				{
					responseContentLength = -1;
				}

				// Open an output stream if set up to download to file.
				if (string.IsNullOrEmpty(settings.DownloadFilePath) == false)
				{
					// First, create the file's sub-directories if applicable.
					string directoryPath = System.IO.Path.GetDirectoryName(settings.DownloadFilePath);
					System.IO.Directory.CreateDirectory(directoryPath);

					// Attempt to create the file to write to.
					outputStream = System.IO.File.Open(
										settings.DownloadFilePath, System.IO.FileMode.Create, System.IO.FileAccess.Write);
				}

				// Fetch the response's HTTP headers.
				var headerCollection = new System.Collections.Generic.Dictionary<string, string>();
				foreach (var headerName in httpWebResponse.Headers.AllKeys)
				{
					headerCollection.Add(headerName, httpWebResponse.Headers[headerName]);
				}
				var readOnlyHeaderCollection =
							new System.Collections.ObjectModel.ReadOnlyDictionary<string, string>(headerCollection);

				// Notify the system that we've started to receive an HTTP response.
				var asynReceivingResponseEventHandler = this.AsyncReceivingResponse;
				if (asynReceivingResponseEventHandler != null)
				{
					asynReceivingResponseEventHandler.Invoke(
							this, new HttpResponseEventArgs((int)httpWebResponse.StatusCode, readOnlyHeaderCollection, null));
				}

				// Do not continue if the handler for the above event has aborted this operation.
				if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
				{
					return;
				}

				// Download the HTTP response's content body.
				var httpBodyBytes = new System.Collections.Generic.List<byte>();
				if (httpWebResponse.ContentLength > 0)
				{
					int bytesReceived = 0;
					var byteBuffer = new byte[2048];
					while (true)
					{
						// Fetch the next batch of response content bytes, if any are left.
						int bytesRead = inputStream.Read(byteBuffer, 0, byteBuffer.Length);
						if (bytesRead == 0)
						{
							// No more bytes are available. The entire response has been received.
							break;
						}

						// Do not continue if this operation has been aborted.
						if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
						{
							return;
						}

						// Copy the HTTP response bytes to file or memory.
						if (outputStream != null)
						{
							// Write the response bytes to file.
							outputStream.Write(byteBuffer, 0, bytesRead);
						}
						else if (bytesRead == byteBuffer.Length)
						{
							// Write the response bytes to the memory buffer.
							// Note: This is an optimization. Copying the entire array in one shot is faster.
							httpBodyBytes.AddRange(byteBuffer);
						}
						else
						{
							// Write the last portion of response bytes to the memory buffer.
							// Note: No more response bytes should be available after this.
							for (int index = 0; index < bytesRead; index++)
							{
								httpBodyBytes.Add(byteBuffer[index]);
							}
						}

						// Notify the system how many bytes have been downloaded so far.
						bytesReceived += bytesRead;
						if (bytesReceived > 0)
						{
							var asyncReceiveProgressChangedEventHandler = this.AsyncReceiveProgressChanged;
							if (asyncReceiveProgressChangedEventHandler != null)
							{
								// Raise a progress event.
								asyncReceiveProgressChangedEventHandler.Invoke(
										this, new HttpProgressEventArgs(bytesReceived, responseContentLength));

								// Do not continue if the handler for the above event has aborted this operation.
								if (object.ReferenceEquals(httpWebRequest, fHttpWebRequest) == false)
								{
									return;
								}
							}
						}
					}
				}
				var immutableHttpBodyBytes = CoronaLabs.WinRT.ImmutableByteBuffer.From(httpBodyBytes);

				// If downloading to file, close its stream.
				if (outputStream != null)
				{
					outputStream.Close();
					outputStream.Dispose();
					outputStream = null;
				}

				// Clear this operation's member variables.
				// This frees up this object to execute another HTTP request.
				fHttpWebRequest = null;
				fClonedSettings = null;

				// Notify the system that we've received the entire response and we're done.
				var asynReceivedResponseEventHandler = this.AsyncReceivedResponse;
				if (asynReceivedResponseEventHandler != null)
				{
					var eventArgs = new HttpResponseEventArgs(
							(int)httpWebResponse.StatusCode, readOnlyHeaderCollection, immutableHttpBodyBytes);
					asynReceivedResponseEventHandler.Invoke(this, eventArgs);
				}
			}
			catch (Exception ex)
			{
				AbortWithError(ex.Message);
				return;
			}
			finally
			{
				// Close all open streams.
				if (inputStream != null)
				{
					try { inputStream.Close(); }
					catch (Exception) { }
				}
				if (outputStream != null)
				{
					try { outputStream.Close(); }
					catch (Exception) { }
				}
				if (httpWebResponse != null)
				{
					try { httpWebResponse.Close(); }
					catch (Exception) { }
				}
			}
		}

		/// <summary>Aborts the currently executing HTTP request.</summary>
		private void OnAbort()
		{
			// Fetch the HTTP request object.
			var httpWebRequest = fHttpWebRequest;
			if (httpWebRequest == null)
			{
				return;
			}

			// Null out the variable the HTTP request operation was using.
			fHttpWebRequest = null;
			fClonedSettings = null;

			// Abort the current HTTP request.
			try
			{
				httpWebRequest.Abort();
			}
			catch (Exception) { }
		}

		/// <summary>
		///  Aborts the currently executing HTTP request and raises an AsynErrorOccurred event with the given message.
		/// </summary>
		/// <param name="message">The error message to be delivered by the AsyncErrorOccurred event. Can be null/empty.</param>
		private void AbortWithError(string message)
		{
			// Abort the current HTTP request first.
			// This is in case the event handler below causes another HTTP request to be executed.
			OnAbort();

			// Use a generic error message if given an empty/null message.
			if (string.IsNullOrEmpty(message))
			{
				message = "Unknown error occurred";
			}

			// Notify the system about the error.
			var asyncErrorOccurredEventHandler = this.AsyncErrorOccurred;
			if (asyncErrorOccurredEventHandler != null)
			{
				asyncErrorOccurredEventHandler.Invoke(this, new CoronaLabs.WinRT.MessageEventArgs(message));
			}
		}

		#endregion
	}
}
