/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/FetchDriver.h"

#include "nsIDOMFile.h"
#include "nsIDocument.h"

#include "nsContentPolicyUtils.h"
#include "nsNetUtil.h"

#include "mozilla/dom/workers/Workers.h"

#include "InternalRequest.h"
#include "InternalResponse.h"

namespace mozilla {
namespace dom {

already_AddRefed<nsIDOMBlob>
InternalResponse::GetBody()
{
  nsRefPtr<nsIDOMBlob> b = mBody->GetBlobInternal(mContentType);
  return b.forget();
}

NS_IMPL_ISUPPORTS(FetchDriver, nsIStreamListener)

FetchDriver::FetchDriver(InternalRequest* aRequest)
  : mRequest(aRequest)
  , mFetchRecursionCount(0)
{
}

FetchDriver::~FetchDriver()
{
}

NS_IMETHODIMP
FetchDriver::Fetch(FetchDriverObserver* aObserver)
{
  workers::AssertIsOnMainThread();
  mObserver = aObserver;

  return Fetch(false /* CORS flag */);
}

NS_IMETHODIMP
FetchDriver::Fetch(bool aCORSFlag)
{
  mFetchRecursionCount++;

  // FIXME(nsm): Deal with HSTS.

  if (!mRequest->ReferrerIsNone()) {
    nsCOMPtr<nsIDocument> doc = mRequest->GetClient();
    MOZ_ASSERT(doc);

    nsString referrer;
    doc->GetReferrer(referrer);

    mRequest->SetReferrer(NS_ConvertUTF16toUTF8(referrer));
  }

  if (!mRequest->IsSynchronous() && mFetchRecursionCount <= 1) {
    nsCOMPtr<nsIRunnable> r =
      NS_NewRunnableMethodWithArg<bool>(this, &FetchDriver::ContinueFetch, aCORSFlag);
    return NS_DispatchToCurrentThread(r);
  }

  return ContinueFetch(aCORSFlag);
}

NS_IMETHODIMP
FetchDriver::ContinueFetch(bool aCORSFlag)
{
  workers::AssertIsOnMainThread();

  // CSP/mixed content checks.
  /*nsCOMPtr<nsIURI> requestURI;
  // FIXME(nsm): Deal with relative URLs.
  nsresult rv = NS_NewURI(getter_AddRefs(requestURI), mRequest->GetURL(),
                          nullptr, nullptr);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return FailWithNetworkError();
  }

  nsCOMPtr<nsIPrincipal> originPrincipal;
  rv = 
  rv = NS_CheckContentLoadPolicy(mRequest->GetContext(),
                                 requestURI,
                                 */

  // Assume same origin for now and do a basic fetch.
  return BasicFetch();
}

NS_IMETHODIMP
FetchDriver::BasicFetch()
{
  nsCString url;
  mRequest->GetURL(url);
  nsCOMPtr<nsIURI> uri;
  nsresult rv = NS_NewURI(getter_AddRefs(uri),
                 url,
                 nullptr,
                 nullptr);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCString scheme;
  rv = uri->GetScheme(scheme);
  NS_ENSURE_SUCCESS(rv, rv);

  if (scheme.LowerCaseEqualsLiteral("about")) {
  } else if (scheme.LowerCaseEqualsLiteral("blob")) {
  } else if (scheme.LowerCaseEqualsLiteral("data")) {
  } else if (scheme.LowerCaseEqualsLiteral("file")) {
  } else if (scheme.LowerCaseEqualsLiteral("http") ||
             scheme.LowerCaseEqualsLiteral("https")) {
    return HttpFetch();
  } else {
    return FailWithNetworkError();
  }

  return NS_ERROR_FAILURE;
}

NS_IMETHODIMP
FetchDriver::HttpFetch(/* FIXME(nsm): Various flags here for CORS */)
{
  mResponse = nullptr;

  // FIXME(nsm): See if ServiceWorker can handle it.
  return ContinueHttpFetchAfterServiceWorker();
}

NS_IMETHODIMP
FetchDriver::ContinueHttpFetchAfterServiceWorker()
{
  if (!mResponse) {
    // FIXME(nsm): Set skip SW flag.
    // FIXME(nsm): Deal with CORS flags cases which will also call
    // ContinueHttpFetchAfterCORSPreflight().
    return ContinueHttpFetchAfterCORSPreflight();
  }

  // Otherwise ServiceWorker replied with a response.
  return ContinueHttpFetchAfterNetworkFetch();
}

NS_IMETHODIMP
FetchDriver::ContinueHttpFetchAfterCORSPreflight()
{
  // mResponse is currently the CORS response.
  // We may have to pass it via argument.
  if (mResponse && mResponse->IsError()) {
    return FailWithNetworkError();
  }

  return HttpNetworkFetch();
}

NS_IMETHODIMP
FetchDriver::HttpNetworkFetch()
{
  nsRefPtr<InternalRequest> httpRequest = new InternalRequest(*mRequest);
  // FIXME(nsm): Figure out how to tee request's body.
  
  // FIXME(nsm): Http network fetch steps 2-7.
  nsresult rv;

  nsCOMPtr<nsIIOService> ios = do_GetIOService(&rv);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCString url;
  httpRequest->GetURL(url);
  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri),
                          url,
                          nullptr,
                          nullptr,
                          ios);
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIChannel> chan;
  rv = NS_NewChannel(getter_AddRefs(chan),
                     uri,
                     ios,
                     nullptr, // FIXME(nsm): Extract loadgroup from request's client.
                     nullptr);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIHttpChannel> httpChan = do_QueryInterface(chan);
  if (httpChan) {
    nsCString method;
    mRequest->GetMethod(method);
    rv = httpChan->SetRequestMethod(method);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  return chan->AsyncOpen(this, nullptr);
}

NS_IMETHODIMP
FetchDriver::ContinueHttpFetchAfterNetworkFetch()
{
  workers::AssertIsOnMainThread();
  MOZ_ASSERT(mResponse);
  MOZ_ASSERT(!mResponse->IsError());

  /*switch (mResponse->GetStatus()) {
    default:
  }*/

  return SucceedWithResponse();
}

NS_IMETHODIMP
FetchDriver::SucceedWithResponse()
{
  mObserver->OnResponseEnd();
  return NS_OK;
}

NS_IMETHODIMP
FetchDriver::FailWithNetworkError()
{
  nsRefPtr<InternalResponse> error = InternalResponse::NetworkError();
  mObserver->OnResponseAvailable(error);
  // FIXME(nsm): Some sort of shutdown?
  return NS_OK;
}

/*NS_IMETHODIMP
FetchDriver::AsyncOpen(FetchDriverObserver* aObserver)
{
  workers::AssertIsOnMainThread();
  mObserver = aObserver;

  // Callers should set the referrer!
  
  nsresult rv;

  nsCOMPtr<nsIIOService> ios = do_GetIOService(&rv);
  NS_ENSURE_SUCCESS(rv, rv);

  DOMString url;
  mRequest->GetUrl(url);
  nsCOMPtr<nsIURI> uri;
  rv = NS_NewURI(getter_AddRefs(uri),
                          url,
                          nullptr,
                          nullptr,
                          ios);
  NS_ENSURE_SUCCESS(rv, rv);
  nsCOMPtr<nsIChannel> chan;
  rv = NS_NewChannel(getter_AddRefs(chan),
                              uri,
                              ios,
                              nullptr, // FIXME(nsm): Extract loadgroup.
                              nullptr);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIHttpChannel> httpChan = do_QueryInterface(chan);
  if (httpChan) {
    nsCString method;
    mRequest->GetMethod(method);
    rv = httpChan->SetRequestMethod(method);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  chan->AsyncOpen(this, nullptr);

  return NS_OK;
}*/

NS_IMETHODIMP
FetchDriver::OnStartRequest(nsIRequest* aRequest,
                            nsISupports* aContext)
{
  nsCOMPtr<nsIHttpChannel> channel = do_QueryInterface(aRequest);
  // For now we only support HTTP.
  MOZ_ASSERT(channel);

  uint32_t status;
  channel->GetResponseStatus(&status);

  nsCString statusText;
  channel->GetResponseStatusText(statusText);
  mResponse = new InternalResponse(status, statusText);
  mObserver->OnResponseAvailable(mResponse);
  
  return NS_OK;
}

/* static */ NS_IMETHODIMP
FetchDriver::StreamReaderFunc(nsIInputStream* aInputStream,
                              void* aClosure,
                              const char* aFragment,
                              uint32_t aToOffset,
                              uint32_t aCount,
                              uint32_t* aWriteCount)
{
  FetchDriver* driver = static_cast<FetchDriver*>(aClosure);
  InternalResponse* response = driver->mResponse;

  nsresult rv = response->mBody->AppendVoidPtr(aFragment, aCount);
  if (NS_SUCCEEDED(rv)) {
    *aWriteCount = aCount;
  }

  return rv;
}

NS_IMETHODIMP
FetchDriver::OnDataAvailable(nsIRequest* aRequest,
                             nsISupports* aContext,
                             nsIInputStream* aInputStream,
                             uint64_t aOffset,
                             uint32_t aCount)
{
  uint32_t aRead;
  MOZ_ASSERT(mResponse);
  if (!mResponse->mBody) {
    mResponse->mBody = new BlobSet();
  }

  nsresult rv = aInputStream->ReadSegments(FetchDriver::StreamReaderFunc,
                                           static_cast<void*>(this),
                                           aCount, &aRead);
  return rv;
}

NS_IMETHODIMP
FetchDriver::OnStopRequest(nsIRequest* aRequest,
                           nsISupports* aContext,
                           nsresult aStatusCode)
{
  nsCOMPtr<nsIChannel> chan = do_QueryInterface(aRequest);
  MOZ_ASSERT(chan);
  chan->GetContentType(mResponse->mContentType);

  ContinueHttpFetchAfterNetworkFetch();
  return NS_OK;
}

} // namespace dom
} // namespace mozilla
