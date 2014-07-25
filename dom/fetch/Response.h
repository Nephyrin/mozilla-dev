/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_Response_h
#define mozilla_dom_Response_h

#include "mozilla/dom/ResponseBinding.h"
#include "mozilla/dom/UnionTypes.h"

#include "nsWrapperCache.h"
#include "nsISupportsImpl.h"

class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class Headers;

class Response MOZ_FINAL : public nsISupports
                         , public nsWrapperCache
{
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(Response)

public:
  Response(nsISupports* aOwner);

  ResponseType
  Type() const
  {
    return mType;
  }

  void
  GetUrl(DOMString& aUrl) const
  {
    aUrl.AsAString() = mUrl;
  }

  uint16_t
  Status() const
  {
    return mStatus;
  }

  void
  GetStatusText(nsCString& aStatusText) const
  {
    aStatusText = mStatusText;
  }

  already_AddRefed<Headers>
  Headers_() const;

  static already_AddRefed<Response>
  Redirect(const GlobalObject& aGlobal, const nsAString& aUrl, uint16_t aStatus);

  already_AddRefed<FetchBodyStream>
  Body() const;

  static already_AddRefed<Response>
  Constructor(const GlobalObject& aGlobal,
              const Optional<ArrayBufferOrArrayBufferViewOrBlobOrString>& aBody,
              const ResponseInit& aInit, ErrorResult& rv);

  virtual JSObject*
  WrapObject(JSContext* aCx)
  {
    return mozilla::dom::ResponseBinding::Wrap(aCx, this);
  }

  nsISupports* GetParentObject() const
  {
    return mOwner;
  }

private:
  ~Response();

  nsISupports* mOwner;

  ResponseType mType;
  nsString mUrl;
  uint16_t mStatus;
  nsCString mStatusText;
  nsRefPtr<Headers> mHeaders;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_Response_h
