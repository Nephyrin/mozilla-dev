/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_FetchEvent_h
#define mozilla_dom_FetchEvent_h

#include "nsIInputStream.h"

#include "mozilla/dom/FetchEventBinding.h"
#include "mozilla/dom/Request.h"
#include "mozilla/dom/Response.h"
#include "mozilla/dom/workers/bindings/URL.h"
#include "Event.h"

class nsPIDOMWindow;
class nsIAlternateSourceChannel;
class nsIChannel;

namespace mozilla {
namespace dom {

class Promise;
class URL;
class Request;
class Response;

class FetchEvent MOZ_FINAL : public Event {
public:
  FetchEvent(mozilla::dom::EventTarget* aOwner,
             nsPresContext *aPresContext,
             mozilla::WidgetEvent* aEvent,
             nsIAlternateSourceChannel* aChannel);

  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(FetchEvent, Event)
  NS_FORWARD_TO_EVENT

  virtual JSObject*
  WrapObject(JSContext* aCx) MOZ_OVERRIDE
  {
    return FetchEventBinding_workers::Wrap(aCx, this);
  }

  nsPIDOMWindow* GetParentObject() const
  {
    return nullptr;
  }

  already_AddRefed<Request> Request_();
  RequestType Type();
  bool IsTopLevel();
  bool IsReload();

  void RespondWith(Promise& r);
  void RespondWith(Response& r);
  already_AddRefed<Promise> ForwardTo(workers::URL& url);
  already_AddRefed<Promise> ForwardTo(const nsAString& url);

  static already_AddRefed<FetchEvent>
  Constructor(mozilla::dom::GlobalObject&, const nsAString& aType,
              const FetchEventInit& aOptions, mozilla::ErrorResult&);

  already_AddRefed<Promise> GetResponsePromise();

private:
  ~FetchEvent();

  bool mIsReload;
  bool mIsTopLevel;
  nsRefPtr<Request> mRequest;
  RequestType mType;
  nsRefPtr<Promise> mResponsePromise;
};

}
}

#endif // mozilla_dom_FetchEvent_h
