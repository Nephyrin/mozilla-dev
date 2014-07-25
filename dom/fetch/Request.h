/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_Request_h
#define mozilla_dom_Request_h

#include "mozilla/dom/RequestBinding.h"
#include "mozilla/dom/UnionTypes.h"

#include "nsWrapperCache.h"
#include "nsISupportsImpl.h"

class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class FetchBodyStream;

class Request MOZ_FINAL : public nsISupports
                        , public nsWrapperCache
{
NS_DECL_CYCLE_COLLECTING_ISUPPORTS
NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Request)

public:
  Request(nsISupports* aOwner);

  void
  GetUrl(DOMString& aUrl) const
  {
    aUrl.AsAString() = mUrl;
  }

  void
  GetMethod(nsCString& aMethod)
  {
    aMethod = mMethod;
  }

  RequestMode
  Mode() const
  {
    return mMode;
  }

  RequestCredentials
  Credentials() const
  {
    return mCredentials;
  }

  void
  GetReferrer(DOMString& aReferrer) const
  {
    aReferrer.AsAString() = mReferrer;
  }

  void GetHeader(const nsAString& header, DOMString& value) const;
  already_AddRefed<Headers> Headers() const;
  already_AddRefed<FetchBodyStream> Body() const;

  static already_AddRefed<Request>
  Constructor(const GlobalObject& global, const RequestOrString& aInput,
              const RequestInit& aInit, ErrorResult& rv);

  virtual JSObject*
  WrapObject(JSContext* aCx)
  {
    return mozilla::dom::RequestBinding::Wrap(aCx, this);
  }

  nsISupports* GetParentObject() const
  {
    return mOwner;
  }

private:
  ~Request();

  nsISupports* mOwner;

  nsCString mMethod;
  nsString mUrl;
  // FIXME(nsm): Headers? Or should they go in InternalRequest?
  RequestMode mMode;
  RequestCredentials mCredentials;
  nsString mReferrer;
  nsRefPtr<FetchBodyStream> mBody;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_Request_h
