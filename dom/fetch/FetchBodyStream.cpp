/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FetchBodyStream.h"

#include "nsIDOMFile.h"
#include "nsISupportsImpl.h"
#include "nsPIDOMWindow.h"

#include "mozilla/dom/Promise.h"
#include "mozilla/dom/workers/bindings/FileReaderSync.h"

#include "File.h" // from workers

using namespace mozilla::dom;

NS_IMPL_CYCLE_COLLECTING_ADDREF(FetchBodyStream)
NS_IMPL_CYCLE_COLLECTING_RELEASE(FetchBodyStream)
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(FetchBodyStream)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(FetchBodyStream)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

FetchBodyStream::FetchBodyStream(nsISupports* aOwner)
  : mOwner(aOwner)
{
  SetIsDOMBinding();
}

FetchBodyStream::~FetchBodyStream()
{
}

/* static */ already_AddRefed<FetchBodyStream>
FetchBodyStream::Constructor(const GlobalObject& aGlobal, ErrorResult& aRv)
{
  nsRefPtr<FetchBodyStream> stream = new FetchBodyStream(aGlobal.GetAsSupports());
  return stream.forget();
}

already_AddRefed<Promise>
FetchBodyStream::AsArrayBuffer()
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(GetParentObject());
  MOZ_ASSERT(global);
  ErrorResult result;
  nsRefPtr<Promise> promise = Promise::Create(global, result);
  if (result.Failed()) {
    return nullptr;
  }

  promise->MaybeReject(NS_ERROR_NOT_AVAILABLE);
  return promise.forget();
}

already_AddRefed<Promise>
FetchBodyStream::AsBlob()
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(GetParentObject());
  MOZ_ASSERT(global);
  ErrorResult result;
  nsRefPtr<Promise> promise = Promise::Create(global, result);
  if (result.Failed()) {
    return nullptr;
  }

  if (mBlob) {
    ThreadsafeAutoJSContext cx;
    JS::Rooted<JS::Value> val(cx);
    nsresult rv = nsContentUtils::WrapNative(cx, mBlob, &val);
    if (NS_FAILED(rv)) {
      promise->MaybeReject(rv);
    } else {
      promise->MaybeResolve(cx, val);
    }
  } else {
    // FIXME(nsm): Not ready to be async yet.
    MOZ_CRASH();
  }
  return promise.forget();
}

already_AddRefed<Promise>
FetchBodyStream::AsFormData()
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(GetParentObject());
  MOZ_ASSERT(global);
  ErrorResult result;
  nsRefPtr<Promise> promise = Promise::Create(global, result);
  if (result.Failed()) {
    return nullptr;
  }

  promise->MaybeReject(NS_ERROR_NOT_AVAILABLE);
  return promise.forget();
}

already_AddRefed<Promise>
FetchBodyStream::AsJSON()
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(GetParentObject());
  MOZ_ASSERT(global);
  ErrorResult result;
  nsRefPtr<Promise> promise = Promise::Create(global, result);
  if (result.Failed()) {
    return nullptr;
  }

  promise->MaybeReject(NS_ERROR_NOT_AVAILABLE);
  return promise.forget();
}

already_AddRefed<Promise>
FetchBodyStream::AsText()
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(GetParentObject());
  MOZ_ASSERT(global);
  ErrorResult result;
  nsRefPtr<Promise> promise = Promise::Create(global, result);
  if (result.Failed()) {
    return nullptr;
  }

  if (mBlob) {
    ThreadsafeAutoJSContext cx;
    JS::Rooted<JSObject*> asObject(cx);
    if (NS_IsMainThread()) {
      JS::Rooted<JS::Value> val(cx);
      result = nsContentUtils::WrapNative(cx, mBlob, &val);
      asObject = val.toObjectOrNull();
    } else {
      asObject = workers::file::CreateBlob(cx, mBlob);
    }

    // Only on worker for now, because i don't want to add the check.
    MOZ_ASSERT(!NS_IsMainThread());
    nsRefPtr<workers::FileReaderSync> reader = new workers::FileReaderSync();

    nsString body;

    nsString encodingStr = NS_LITERAL_STRING("UTF-8");
    Optional<nsAString> encoding;
    encoding = &encodingStr;
    reader->ReadAsText(asObject, encoding, body, result);
    if (result.Failed()) {
      promise->MaybeReject(result.ErrorCode());
    } else {
      promise->MaybeResolve(body);
    }
  } else {
    // FIXME(nsm): Not ready to be async yet.
    MOZ_CRASH();
  }
  return promise.forget();
}
