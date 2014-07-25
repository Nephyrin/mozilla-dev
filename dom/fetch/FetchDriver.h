/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FetchDriver_h
#define mozilla_dom_FetchDriver_h

#include "nsIStreamListener.h"
#include "nsDOMBlobBuilder.h"

class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class InternalRequest;
class InternalResponse;

class FetchDriverObserver
{
public:
  NS_INLINE_DECL_REFCOUNTING(FetchDriverObserver);
  virtual void OnResponseAvailable(InternalResponse* aResponse) = 0;
  virtual void OnResponseEnd() = 0;

protected:
  virtual ~FetchDriverObserver()
  { };
};

class FetchDriver MOZ_FINAL : public nsIStreamListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSISTREAMLISTENER

  explicit FetchDriver(InternalRequest* aRequest);
  NS_IMETHOD Fetch(FetchDriverObserver* aObserver);
  // NS_IMETHOD AsyncOpen(FetchDriverObserver* aObserver);

private:
  nsRefPtr<InternalRequest> mRequest;
  nsRefPtr<InternalResponse> mResponse;
  nsRefPtr<FetchDriverObserver> mObserver;
  uint32_t mFetchRecursionCount;

  FetchDriver() MOZ_DELETE;
  FetchDriver(const FetchDriver&) MOZ_DELETE;
  FetchDriver& operator=(const FetchDriver&) MOZ_DELETE;
  ~FetchDriver();

  NS_IMETHOD Fetch(bool aCORSFlag);
  NS_IMETHOD ContinueFetch(bool aCORSFlag);
  NS_IMETHOD BasicFetch();
  NS_IMETHOD HttpFetch();
  NS_IMETHOD ContinueHttpFetchAfterServiceWorker();
  NS_IMETHODIMP ContinueHttpFetchAfterCORSPreflight();
  NS_IMETHODIMP HttpNetworkFetch();
  NS_IMETHODIMP ContinueHttpFetchAfterNetworkFetch();
  NS_IMETHODIMP FailWithNetworkError();
  NS_IMETHODIMP SucceedWithResponse();

  static nsresult
  StreamReaderFunc(nsIInputStream* aInputStream,
                              void* aClosure,
                              const char* aFragment,
                              uint32_t aToOffset,
                              uint32_t aCount,
                              uint32_t* aWriteCount);
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FetchDriver_h
