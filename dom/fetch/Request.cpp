/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Request.h"
#include "nsDOMString.h"
#include "nsPIDOMWindow.h"
#include "nsIURI.h"
#include "nsISupportsImpl.h"

#include "mozilla/dom/FetchBodyStream.h"

using namespace mozilla::dom;

NS_IMPL_CYCLE_COLLECTING_ADDREF(Request)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Request)
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Request)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Request)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

Request::Request(nsISupports* aOwner)
  : mOwner(aOwner)
{
  SetIsDOMBinding();
}

Request::~Request()
{
}

void
Request::GetHeader(const nsAString& header, DOMString& value) const
{
  MOZ_CRASH("NOT IMPLEMENTED!");
}

already_AddRefed<Headers>
Request::Headers() const
{
  MOZ_CRASH("NOT IMPLEMENTED!");
}

already_AddRefed<FetchBodyStream>
Request::Body() const
{
  MOZ_CRASH("NOT IMPLEMENTED!");
}

/*static*/ already_AddRefed<Request>
Request::Constructor(const GlobalObject& global, const RequestOrString& aInput,
                     const RequestInit& aInit, ErrorResult& rv)
{
  nsRefPtr<Request> request = new Request(global.GetAsSupports());
  request->mMethod = aInit.mMethod.WasPassed() ? aInit.mMethod.Value() : NS_LITERAL_CSTRING("GET");
  // FIXME(nsm): Headers
  // FIXME(nsm): Body setup from FetchBodyStreamInit.
  request->mMode = aInit.mMode.WasPassed() ? aInit.mMode.Value() : RequestMode::Same_origin;
  request->mCredentials = aInit.mCredentials.WasPassed() ? aInit.mCredentials.Value() : RequestCredentials::Omit;
  return request.forget();
}
