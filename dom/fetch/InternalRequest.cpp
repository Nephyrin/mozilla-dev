/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "InternalRequest.h"

#include "nsIDocument.h"

#include "mozilla/dom/ScriptSettings.h"
#include "mozilla/dom/workers/Workers.h"

namespace mozilla {
namespace dom {

InternalRequest::InternalRequest(nsIDocument* aClient)
  : mMethod("GET")
  //, mUnsafeRequest(false)
  , mPreserveContentCodings(false)
  , mClient(aClient)
  , mSkipServiceWorker(false)
  //, mContextFrameType(NONE)
  //, mForceOriginHeader(false)
  //, mSameOriginDataURL(false)
  , mReferrerType(REFERRER_CLIENT)
  //, mAuthenticationFlag(false)
  , mSynchronous(false)
  , mMode(RequestMode::No_cors)
  , mCredentialsMode(RequestCredentials::Omit)
  //, mUseURLCredentials(false)
  //, mManualRedirect(false)
  //, mRedirectCount(0)
  //, mResponseTainting(RESPONSETAINT_BASIC)
{
}

already_AddRefed<InternalRequest>
InternalRequest::GetRestrictedCopy(nsIDocument* aGlobal)
{
  workers::AssertIsOnMainThread(); // Required?
  nsRefPtr<InternalRequest> copy = new InternalRequest(aGlobal);
  copy->mURL.Assign(mURL);
  copy->SetMethod(mMethod);
  // FIXME(nsm): Headers list.
  // FIXME(nsm): Tee body.
  copy->mPreserveContentCodings = true;
  
  nsIURI* uri = aGlobal->GetDocumentURI();
  if (!uri) {
    return nullptr;
  }

  nsContentUtils::GetASCIIOrigin(uri, copy->mOrigin);

  copy->SetReferrer(mClient);
  // FIXME(nsm): Set context;

  copy->mMode = mMode;
  copy->mCredentialsMode = mCredentialsMode;
  return copy.forget();
}

InternalRequest::~InternalRequest()
{
}

void
InternalRequest::SetReferrer(nsIDocument* aClient)
{
  mReferrerType = REFERRER_CLIENT;
  mReferrerClient = aClient;
}

} // namespace dom
} // namespace mozilla
