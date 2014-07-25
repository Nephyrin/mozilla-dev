/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Response.h"
#include "nsDOMString.h"
#include "nsPIDOMWindow.h"
#include "nsIURI.h"
#include "nsISupportsImpl.h"

using namespace mozilla::dom;

NS_IMPL_CYCLE_COLLECTING_ADDREF(Response)
NS_IMPL_CYCLE_COLLECTING_RELEASE(Response)
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(Response)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Response)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

Response::Response(nsISupports* aOwner)
  : mOwner(aOwner)
{
  SetIsDOMBinding();
}

Response::~Response()
{
}

already_AddRefed<Headers>
Response::Headers_() const
{
  MOZ_CRASH("NOT IMPLEMENTED!");
}

/* static */ already_AddRefed<Response>
Response::Redirect(const GlobalObject& aGlobal, const nsAString& aUrl,
                   uint16_t aStatus)
{
  MOZ_CRASH("NOT IMPLEMENTED!");
}

already_AddRefed<FetchBodyStream>
Response::Body() const
{
  MOZ_ASSERT(mBody);
  nsRefPtr<FetchBodyStream> body = mBody;
  return body.forget();
}

/*static*/ already_AddRefed<Response>
Response::Constructor(const GlobalObject& global,
                      const Optional<ArrayBufferOrArrayBufferViewOrBlobOrString>& aBody,
                      const ResponseInit& aInit, ErrorResult& rv)
{
  nsRefPtr<Response> response = new Response(global.GetAsSupports());
  response->mStatus = aInit.mStatus;
  response->mStatusText = aInit.mStatusText.WasPassed() ? aInit.mStatusText.Value() : NS_LITERAL_CSTRING("OK");
  // FIXME(nsm): Headers and body.
  return response.forget();
}
