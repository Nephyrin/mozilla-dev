/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Fetch.h"

#include "nsIGlobalObject.h"

#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Request.h"
#include "mozilla/dom/Response.h"
#include "mozilla/dom/ScriptSettings.h"
#include "mozilla/dom/workers/Workers.h"

#include "InternalResponse.h"

namespace mozilla {
namespace dom {

already_AddRefed<Promise>
DOMFetch(nsIGlobalObject* aGlobal, const RequestOrString& aInput,
         const RequestInit& aInit, ErrorResult& aRv)
{
  nsRefPtr<Promise> p = Promise::Create(aGlobal, aRv);
  if (aRv.Failed()) {
    return nullptr;
  }

  AutoJSAPI jsapi;
  jsapi.Init(aGlobal);
  JSContext* cx = jsapi.cx();

  JS::Rooted<JSObject*> jsGlobal(cx, aGlobal->GetGlobalJSObject());
  GlobalObject global(cx, jsGlobal);

  nsRefPtr<Request> request = Request::Constructor(global, aInput, aInit, aRv);
  if (aRv.Failed()) {
    return nullptr;
  }

  nsRefPtr<mozilla::dom::InternalRequest> r = request->GetInternalRequest();

  nsRefPtr<ResolveFetchWithResponse> resolver = new ResolveFetchWithResponse(p);
  nsRefPtr<FetchDriver> fetch = new FetchDriver(r);
  aRv = fetch->Fetch(resolver);
  if (aRv.Failed()) {
    return nullptr;
  }

  return p.forget();
}

ResolveFetchWithResponse::ResolveFetchWithResponse(Promise* aPromise)
  : mPromise(aPromise)
{
}

void
ResolveFetchWithResponse::OnResponseAvailable(InternalResponse* aResponse)
{
  NS_ASSERT_OWNINGTHREAD(ResolveFetchWithResponse)
  mozilla::dom::workers::AssertIsOnMainThread();
  mInternalResponse = aResponse;

  ResponseInit init;
  init.mStatus = aResponse->GetStatus();
  init.mStatusText.Construct(aResponse->GetStatusText());

  nsCOMPtr<nsIGlobalObject> go = mPromise->GetParentObject();
  AutoSafeJSContext cx;
  JS::Rooted<JSObject*> jsGlobal(cx, go->GetGlobalJSObject());
  GlobalObject global(cx, jsGlobal);

  Optional<ArrayBufferOrArrayBufferViewOrBlobOrString> unusedBody;
  ErrorResult rv;
  mDOMResponse = Response::Constructor(global, unusedBody, init, rv);
  if (rv.Failed()) {
    mPromise->MaybeReject(rv.ErrorCode());
    return;
  }

  mPromise->MaybeResolve(mDOMResponse);
}

void
ResolveFetchWithResponse::OnResponseEnd()
{
  NS_ASSERT_OWNINGTHREAD(ResolveFetchWithResponse)
  mozilla::dom::workers::AssertIsOnMainThread();
  nsCOMPtr<nsIDOMBlob> blobBody = mInternalResponse->GetBody();
  if (blobBody) {
    // FIXME(nsm): Encapsulate.
    mDOMResponse->SetBody(blobBody);
  }
}

ResolveFetchWithResponse::~ResolveFetchWithResponse()
{
  NS_ASSERT_OWNINGTHREAD(ResolveFetchWithResponse)
}

} // namespace dom
} // namespace mozilla
