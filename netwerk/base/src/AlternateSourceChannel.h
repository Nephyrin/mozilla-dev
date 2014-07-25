/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_net_AlternateSourceChannel_h
#define mozilla_net_AlternateSourceChannel_h

#include "nsAHttpTransaction.h"
#include "nsIAlternateSourceChannel.h"
#include "nsIChannel.h"
#include "nsIStreamListener.h"
#include "nsCOMPtr.h"
#include "nsAutoPtr.h"
#include "nsString.h"

class nsInputStreamPump;

namespace mozilla {
namespace net {

class AlternateSourceChannel : public nsIChannel
                             , public nsIAlternateSourceChannel
                             , public nsIStreamListener
                             , public nsAHttpSegmentWriter {
public:
  AlternateSourceChannel(nsIChannel* aWrappedChannel);
  virtual ~AlternateSourceChannel();

  NS_DECL_THREADSAFE_ISUPPORTS
  NS_DECL_NSIALTERNATESOURCECHANNEL
  NS_DECL_NSICHANNEL
  NS_DECL_NSIREQUEST
  NS_DECL_NSISTREAMLISTENER
  NS_DECL_NSIREQUESTOBSERVER
  NS_DECL_NSAHTTPSEGMENTWRITER

private:
  nsCOMPtr<nsIChannel> mWrappedChannel;
  nsCOMPtr<nsIStreamListener> mListener;
  nsCOMPtr<nsISupports> mContext;
  nsCOMPtr<nsIInputStream> mBody;
  nsRefPtr<nsInputStreamPump> mPump;
  nsresult mStatus;
  uint32_t mSuspendCount;
  int64_t mContentLength;
  bool mForwardToWrapped;
  bool mCanceled;
  bool mPending;
  bool mWasOpened;
};

} // namespace net
} // namespace mozilla

#endif // mozilla_net_AlternateSourceChannel_h
