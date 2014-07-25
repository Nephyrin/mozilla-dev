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

} // namespace dom
} // namespace mozilla
