/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_InternalRequest_h
#define mozilla_dom_InternalRequest_h

#include "mozilla/dom/RequestBinding.h"
#include "mozilla/dom/UnionTypes.h"

#include "nsIContentPolicy.h"
#include "nsISupportsImpl.h"

class nsIDocument;
class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class FetchBodyStream;
class Request;

class InternalRequest MOZ_FINAL
{
  friend class Request;

public:
  NS_INLINE_DECL_REFCOUNTING(InternalRequest)

  enum ContextFrameType
  {
    AUXILIARY = 0,
    TOP_LEVEL,
    NESTED,
    NONE,
  };

  explicit InternalRequest(nsIDocument* aClient)
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

  // FIXME(nsm): Copy constructor.

  void
  GetMethod(nsCString& aMethod)
  {
    aMethod.Assign(mMethod);
  }

  void
  SetMethod(const nsACString& aMethod)
  {
    mMethod.Assign(aMethod);
  }

  void
  GetURL(nsCString& aURL)
  {
    aURL.Assign(mURL);
  }

  // Should probably restrict this to friends.
  void
  SetURL(const nsACString& aURL)
  {
    mURL.Assign(aURL);
  }

  nsIDocument*
  GetClient()
  {
    return mClient;
  }

  bool
  ReferrerIsNone()
  {
    return mReferrerType == REFERRER_NONE;
  }

  void
  SetReferrer(const nsACString& aReferrer)
  {
    // May be removed later.
    MOZ_ASSERT(!ReferrerIsNone());
    mReferrerType = REFERRER_URL;
    mReferrerURL.Assign(aReferrer);
  }

  void
  SetReferrer(nsIDocument* aClient)
  {
    mReferrerType = REFERRER_CLIENT;
    mReferrerClient = aClient;
  }

  bool
  IsSynchronous()
  {
    return mSynchronous;
  }

  void
  SetMode(RequestMode aMode)
  {
    mMode = aMode;
  }

  void
  SetCredentialsMode(RequestCredentials aCredentialsMode)
  {
    mCredentialsMode = aCredentialsMode;
  }

  nsContentPolicyType
  GetContext()
  {
    return mContext;
  }

  // The global is used as the client for the new object.
  already_AddRefed<InternalRequest>
  GetRestrictedCopy(nsIDocument* aGlobal);

private:
  ~InternalRequest();

  nsCString mMethod;
  nsCString mURL;
  // FIXME(nsm): Do we just use Headers for internal header list?

  //bool mUnsafeRequest;
  // FIXME(nsm): How do we represent body?

  bool mPreserveContentCodings;

  // FIXME(nsm): This could be non-document stuff. What do we do then?
  nsIDocument* mClient;

  bool mSkipServiceWorker;
  nsContentPolicyType mContext;

  //ContextFrameType mContextFrameType;

  // FIXME(nsm): Should the origin be a nsIPrincipal?
  nsCString mOrigin;

  //bool mForceOriginHeader;
  //bool mSameOriginDataURL;

  // Since referrer type can be none, client or a URL.
  enum {
    REFERRER_NONE = 0,
    REFERRER_CLIENT,
    REFERRER_URL,
  } mReferrerType;
  nsCString mReferrerURL;
  nsCOMPtr<nsIDocument> mReferrerClient;

  //bool mAuthenticationFlag;
  bool mSynchronous;
  RequestMode mMode;
  RequestCredentials mCredentialsMode;
  //bool mUseURLCredentials;
  //bool mManualRedirect;
  //uint32_t mRedirectCount;
  /*enum {
    RESPONSETAINT_BASIC,
    RESPONSETAINT_CORS,
    RESPONSETAINT_OPAQUE,
  } mResponseTainting;*/
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_InternalRequest_h
