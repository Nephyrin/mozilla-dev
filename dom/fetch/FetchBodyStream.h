/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FetchBodyStream_h
#define mozilla_dom_FetchBodyStream_h

#include "mozilla/dom/FetchBinding.h"

#include "nsISupportsImpl.h"
#include "nsWrapperCache.h"

class nsPIDOMWindow;

namespace mozilla {
namespace dom {

class Promise;

class FetchBodyStream : public nsISupports
                      , public nsWrapperCache
{
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(FetchBodyStream)

public:
  FetchBodyStream(nsISupports* aOwner);
  FetchBodyStream(const FetchBodyStream& aOther)
  {
  }

  already_AddRefed<Promise>
  AsArrayBuffer();

  already_AddRefed<Promise>
  AsBlob();

  already_AddRefed<Promise>
  AsFormData();

  already_AddRefed<Promise>
  AsJSON();

  already_AddRefed<Promise>
  AsText();

  static already_AddRefed<FetchBodyStream>
  Constructor(const GlobalObject& aGlobal, ErrorResult& aRv);

  virtual JSObject*
  WrapObject(JSContext* aCx)
  {
    return mozilla::dom::FetchBodyStreamBinding::Wrap(aCx, this);
  }

  nsISupports* GetParentObject() const
  {
    return mOwner;
  }

private:
  virtual ~FetchBodyStream();

  nsISupports* mOwner;
};

} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_FetchBodyStream_h
