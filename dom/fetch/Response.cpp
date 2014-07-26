/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "Response.h"
#include "nsDOMBlobBuilder.h"
#include "nsDOMString.h"
#include "nsPIDOMWindow.h"
#include "nsIURI.h"
#include "nsISupportsImpl.h"
#include "nsIDOMFile.h"

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

Response::Response(const Response& aOther)
  : Response(aOther.mOwner)
{
  mType = aOther.mType;
  mUrl = aOther.mUrl;
  mStatus = aOther.mStatus;
  mStatusText = aOther.mStatusText;
  mBody = aOther.mBody;
  // We don't copy headers yet.
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

  if (aBody.WasPassed() && aBody.Value().IsString()) {
    nsString body;
    body.Assign(aBody.Value().GetAsString());
    nsAutoPtr<BlobSet> blobBuilder(new BlobSet());
    nsCString foo = NS_ConvertUTF16toUTF8(body);
    blobBuilder->AppendVoidPtr(foo.get(), foo.Length());
    response->mBody = new FetchBodyStream(global.GetAsSupports());
    nsCOMPtr<nsIDOMBlob> blob = blobBuilder->GetBlobInternal(NS_LITERAL_CSTRING("text/plain"));
    response->mBody->SetBlob(blob);
  }
  // FIXME(nsm): Headers and body.
  return response.forget();
}
