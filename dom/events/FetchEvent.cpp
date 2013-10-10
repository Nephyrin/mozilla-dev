/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "FetchEvent.h"
#include "nsIAlternateSourceChannel.h"
#include "nsIChannel.h"
#include "WorkerPrivate.h"
#include "WorkerScope.h"
#include "mozilla/dom/Response.h"
#include "mozilla/dom/Promise.h"

using namespace mozilla::dom;

NS_IMPL_CYCLE_COLLECTION_INHERITED(FetchEvent, Event,
                                   mRequest, mResponsePromise)

NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION_INHERITED(FetchEvent)
NS_INTERFACE_MAP_END_INHERITING(Event)

NS_IMPL_ADDREF_INHERITED(FetchEvent, Event)
NS_IMPL_RELEASE_INHERITED(FetchEvent, Event)

FetchEvent::FetchEvent(mozilla::dom::EventTarget* aOwner,
                       nsPresContext *aPresContext,
                       mozilla::WidgetEvent* aEvent,
                       nsIAlternateSourceChannel *aChannel)
: Event(aOwner, aPresContext, aEvent)
{
}

FetchEvent::~FetchEvent()
{
}

already_AddRefed<Request>
FetchEvent::Request_()
{
  nsRefPtr<Request> r = mRequest;
  return r.forget();
}

RequestType
FetchEvent::Type()
{
  return mType;
}

bool
FetchEvent::IsTopLevel()
{
  return mIsTopLevel;
}

bool
FetchEvent::IsReload()
{
  return mIsReload;
}

already_AddRefed<Promise>
FetchEvent::GetResponsePromise()
{
  MOZ_ASSERT(DefaultPrevented());
  nsRefPtr<Promise> p = mResponsePromise;
  return p.forget();
}

void
FetchEvent::RespondWith(Promise& r)
{
  PreventDefault();
  if (mResponsePromise) {
    return;
  }

  mResponsePromise = &r;
}

void
FetchEvent::RespondWith(Response& r)
{
  workers::WorkerPrivate* worker = workers::GetCurrentThreadWorkerPrivate();
  MOZ_ASSERT(worker);
  worker->AssertIsOnWorkerThread();

  PreventDefault();
  if (mResponsePromise) {
    return;
  }

  nsIGlobalObject* global = worker->GlobalScope();
  fprintf(stderr, "NSM FetchEvent::RespondWith Response\n");

  ErrorResult result;
  mResponsePromise = Promise::Create(global, result);
  MOZ_ASSERT(!result.Failed());
  mResponsePromise->MaybeResolve(&r);
}

already_AddRefed<Promise>
FetchEvent::ForwardTo(workers::URL& url)
{
  return nullptr;
}

already_AddRefed<Promise>
FetchEvent::ForwardTo(const nsAString& url)
{
  return nullptr;
}

/*static*/ already_AddRefed<FetchEvent>
FetchEvent::Constructor(mozilla::dom::GlobalObject& aGlobal, const nsAString& aType,
                        const FetchEventInit& aOptions, mozilla::ErrorResult&)
{
  nsCOMPtr<EventTarget> owner = do_QueryInterface(aGlobal.GetAsSupports());
  nsRefPtr<FetchEvent> e = new FetchEvent(owner, nullptr, nullptr, nullptr);
  bool trusted = e->Init(owner);
  e->InitEvent(aType, aOptions.mBubbles, aOptions.mCancelable);
  e->SetTrusted(trusted);

  e->mIsReload = aOptions.mIsReload;
  e->mIsTopLevel = aOptions.mIsTopLevel;
  e->mRequest = aOptions.mRequest;
  e->mType = aOptions.mType;
  return e.forget();
}
