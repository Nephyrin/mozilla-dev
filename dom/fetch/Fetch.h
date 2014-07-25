/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_Fetch_h
#define mozilla_dom_Fetch_h

#include "nsISupportsImpl.h"

#include "FetchDriver.h"

#include "mozilla/dom/RequestBinding.h"
#include "mozilla/dom/UnionTypes.h"

class nsIGlobalObject;

namespace mozilla {
namespace dom {

class InternalResponse;
class Promise;
class Response;

class ResolveFetchWithResponse MOZ_FINAL : public FetchDriverObserver
{
  nsRefPtr<Promise> mPromise;
  nsRefPtr<InternalResponse> mInternalResponse;
  nsRefPtr<Response> mDOMResponse;

  NS_DECL_OWNINGTHREAD
public:
  ResolveFetchWithResponse(Promise* aPromise);

  void
  OnResponseAvailable(InternalResponse* aResponse) MOZ_OVERRIDE;

  void
  OnResponseEnd() MOZ_OVERRIDE;

private:
  ~ResolveFetchWithResponse();
};

// Utility since windows and workers implement the same fetch() initialization
// logic.
already_AddRefed<Promise>
DOMFetch(nsIGlobalObject* aGlobal, const RequestOrString& aInput,
         const RequestInit& aInit, ErrorResult& aRv);


} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_Fetch_h
