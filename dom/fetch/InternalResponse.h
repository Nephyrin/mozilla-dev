/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_InternalResponse_h
#define mozilla_dom_InternalResponse_h

#include "nsISupportsImpl.h"

namespace mozilla {
namespace dom {

class InternalResponse MOZ_FINAL
{
  friend class FetchDriver;

  ~InternalResponse()
  { }

  uint32_t mStatus;
  const nsCString mStatusText;
  nsCString mContentType;
  nsAutoPtr<BlobSet> mBody;
  bool mError;

public:
  NS_INLINE_DECL_REFCOUNTING(InternalResponse)

  InternalResponse(uint32_t aStatus, const nsACString& aStatusText)
    : mStatus(aStatus), mStatusText(aStatusText), mError(false)
  { }

  static already_AddRefed<InternalResponse>
  NetworkError()
  {
    nsRefPtr<InternalResponse> response = new InternalResponse(0, NS_LITERAL_CSTRING(""));
    response->mError = true;
    return response.forget();
  }

  bool
  IsError()
  {
    return mError;
  }

  uint32_t
  GetStatus()
  {
    return mStatus;
  }

  const nsCString&
  GetStatusText()
  {
    return mStatusText;
  }

  already_AddRefed<nsIDOMBlob>
  GetBody();
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_InternalResponse_h
