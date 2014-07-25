/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/net/NeckoParent.h"
#include "nsIHttpChannelInternal.h"
#include "nsIHttpChannelChild.h"
#include "nsILoadInfo.h"
#include "nsInputStreamPump.h"
#include "nsIOService.h"
#include "AlternateSourceChannel.h"
#include "nsNetUtil.h"
#include "nsSocketTransportService2.h"
#include "nsHttpTransaction.h"
#include "nsThreadUtils.h"

class nsIInterfaceRequestor;

namespace mozilla {
namespace net {

NS_IMPL_ADDREF(AlternateSourceChannel)
NS_IMPL_RELEASE(AlternateSourceChannel)

NS_IMPL_QUERY_HEAD(AlternateSourceChannel)
  NS_IMPL_QUERY_BODY(nsIChannel)
  NS_IMPL_QUERY_BODY(nsIAlternateSourceChannel)
  NS_IMPL_QUERY_BODY(nsIStreamListener)
  NS_IMPL_QUERY_BODY(nsINetworklessChannelListener)
  NS_IMPL_QUERY_BODY_AMBIGUOUS(nsISupports, nsIChannel)
NS_IMPL_QUERY_TAIL_USING_AGGREGATOR(mWrappedChannel)

AlternateSourceChannel::AlternateSourceChannel(nsIChannel* aWrappedChannel,
                                               nsIAlternateSourceChannelListener* aCallback)
: mWrappedChannel(aWrappedChannel)
, mCallback(aCallback)
, mStatus(NS_OK)
, mForwardToWrapped(false)
, mCanceled(false)
, mPending(false)
, mWasOpened(false)
{
    MOZ_ASSERT(mWrappedChannel);
}

AlternateSourceChannel::~AlternateSourceChannel()
{
}

NS_IMETHODIMP
AlternateSourceChannel::GetWrappedChannel(nsIChannel** aChannel)
{
  NS_ADDREF(*aChannel = mWrappedChannel);
  return NS_OK;
}

NS_IMETHODIMP
AlternateSourceChannel::ForwardToOriginalChannel()
{
  if (mForwardToWrapped) {
    return NS_OK;
  }

  mForwardToWrapped = true;
  nsCOMPtr<nsIHttpChannelInternal> httpChan = do_QueryInterface(mWrappedChannel);
  if (httpChan) {
    httpChan->AsyncOpenFinish();
  } else {
    mWrappedChannel->AsyncOpen(mListener, mContext);
    mListener = nullptr;
    mContext = nullptr;
  }
  return NS_OK;
}

class WriteSegmentsRunnable : public nsRunnable {
public:
    WriteSegmentsRunnable(nsHttpTransaction* aTransaction,
                          AlternateSourceChannel* aWriter)
    : mTransaction(aTransaction)
    , mWriter(aWriter)
    {
    }

    NS_IMETHOD Run()
    {
        MOZ_ASSERT(PR_GetCurrentThread() == gSocketThread);

        uint32_t n;
        nsresult rv = mTransaction->WriteSegments(mWriter, nsIOService::gDefaultSegmentSize, &n);
        if (NS_FAILED(rv) || n == 0) {
            nsresult reason = rv == NS_BASE_STREAM_CLOSED ? NS_OK : rv;
            mTransaction->Close(reason);
            return NS_OK;
        }
        rv = NS_DispatchToCurrentThread(this);
        NS_ENSURE_SUCCESS(rv, rv);
        return NS_OK;
    }

private:
    nsRefPtr<nsHttpTransaction> mTransaction;
    nsRefPtr<AlternateSourceChannel> mWriter;
};

nsresult
AlternateSourceChannel::OnWriteSegment(char* aBuf, uint32_t aCount, uint32_t* aCountWritten)
{
    if (aCount == 0) {
        // some WriteSegments implementations will erroneously call the reader
        // to provide 0 bytes worth of data.  we must protect against this case
        // or else we'd end up closing the socket prematurely.
        NS_ERROR("bad WriteSegments implementation");
        return NS_ERROR_FAILURE; // stop iterating
    }
    nsresult rv = mBody->Read(aBuf, aCount, aCountWritten);

    if (NS_FAILED(rv)) {
      NS_WARNING("AlternateSourceChannel::OnWriteSegment failed to write!");
      return rv;
    } else if (*aCountWritten == 0) {
      // done.
      return NS_BASE_STREAM_CLOSED;
    }
    return NS_OK;
}

NS_IMETHODIMP
AlternateSourceChannel::InitiateAlternateResponse(nsIInputStream* aBody)
{
    mBody = aBody;

    nsCOMPtr<nsIHttpChannelChild> httpChanChild = do_QueryInterface(mWrappedChannel);
    if (httpChanChild) {
        httpChanChild->SynthesizeResponse(aBody);
        return NS_OK;
    }

    nsCOMPtr<nsIHttpChannelInternal> httpChan = do_QueryInterface(mWrappedChannel);
    if (httpChan) {
        nsRefPtr<nsHttpTransaction> transaction;
        nsresult rv = httpChan->GetConnectionlessTransaction(getter_AddRefs(transaction));
        NS_ENSURE_SUCCESS(rv, rv);
        nsCOMPtr<nsIRunnable> event = new WriteSegmentsRunnable(transaction, this);
        rv = gSocketTransportService->Dispatch(event, NS_DISPATCH_NORMAL);
        NS_ENSURE_SUCCESS(rv, rv);
        return NS_OK;
    }

        nsresult rv = nsInputStreamPump::Create(getter_AddRefs(mPump), aBody);
        NS_ENSURE_SUCCESS(rv, rv);
        rv = mPump->AsyncRead(this, nullptr);
        NS_ENSURE_SUCCESS(rv, rv);
    return NS_OK;
}

NS_IMETHODIMP
AlternateSourceChannel::GetOriginalURI(nsIURI** aOriginalURI)
{
  return mWrappedChannel->GetOriginalURI(aOriginalURI);
}

NS_IMETHODIMP
AlternateSourceChannel::SetOriginalURI(nsIURI* aOriginalURI)
{
  return mWrappedChannel->SetOriginalURI(aOriginalURI);
}

NS_IMETHODIMP
AlternateSourceChannel::GetURI(nsIURI** aURI)
{
  return mWrappedChannel->GetURI(aURI);
}

NS_IMETHODIMP
AlternateSourceChannel::GetOwner(nsISupports** aOwner)
{
  return mWrappedChannel->GetOwner(aOwner);
}

NS_IMETHODIMP
AlternateSourceChannel::SetOwner(nsISupports* aOwner)
{
  return mWrappedChannel->SetOwner(aOwner);
}

NS_IMETHODIMP
AlternateSourceChannel::GetNotificationCallbacks(nsIInterfaceRequestor** aCallbacks)
{
  return mWrappedChannel->GetNotificationCallbacks(aCallbacks);
}

NS_IMETHODIMP
AlternateSourceChannel::SetNotificationCallbacks(nsIInterfaceRequestor* aCallbacks)
{
  return mWrappedChannel->SetNotificationCallbacks(aCallbacks);
}

NS_IMETHODIMP
AlternateSourceChannel::GetSecurityInfo(nsISupports** aSecurityInfo)
{
  return mWrappedChannel->GetSecurityInfo(aSecurityInfo);
}

NS_IMETHODIMP
AlternateSourceChannel::GetContentType(nsACString& aContentType)
{
    return mWrappedChannel->GetContentType(aContentType);
}

NS_IMETHODIMP
AlternateSourceChannel::SetContentType(const nsACString& aContentType)
{
    return mWrappedChannel->SetContentType(aContentType);
}

NS_IMETHODIMP
AlternateSourceChannel::GetContentCharset(nsACString& aContentCharset)
{
    return mWrappedChannel->GetContentCharset(aContentCharset);
}

NS_IMETHODIMP
AlternateSourceChannel::SetContentCharset(const nsACString& aContentCharset)
{
    return mWrappedChannel->SetContentCharset(aContentCharset);
}

NS_IMETHODIMP
AlternateSourceChannel::GetContentLength(int64_t* aContentLength)
{
    return mWrappedChannel->GetContentLength(aContentLength);
}

NS_IMETHODIMP
AlternateSourceChannel::SetContentLength(int64_t aContentLength)
{
    return mWrappedChannel->SetContentLength(aContentLength);
}

NS_IMETHODIMP
AlternateSourceChannel::Open(nsIInputStream** aStream)
{
  return NS_ERROR_NOT_AVAILABLE;
}

class ReadyNonHttpChannel : public nsRunnable
{
  nsRefPtr<AlternateSourceChannel> mChannel;

public:
  ReadyNonHttpChannel(AlternateSourceChannel* aChannel)
    : mChannel(aChannel)
  {
    MOZ_ASSERT(NS_IsMainThread());
  }

  NS_IMETHODIMP
  Run()
  {
    MOZ_ASSERT(NS_IsMainThread());
    mChannel->OnNetworklessChannelReady();
    return NS_OK;
  }
};

NS_IMETHODIMP
AlternateSourceChannel::AsyncOpen(nsIStreamListener* aListener, nsISupports* aContext)
{
  NS_ENSURE_TRUE(!mWasOpened, NS_ERROR_FAILURE);

  if (mForwardToWrapped) {
    return mWrappedChannel->AsyncOpen(aListener, aContext);
  }

  mWasOpened = true;

  nsCOMPtr<nsIHttpChannelInternal> httpChan = do_QueryInterface(mWrappedChannel);
  if (httpChan) {
    httpChan->AsyncOpenNetworkless(aListener, aContext, this);
  } else {
    mListener = aListener;
    mContext = aContext;
    nsRefPtr<ReadyNonHttpChannel> r = new ReadyNonHttpChannel(this);
    NS_DispatchToCurrentThread(r);
  }
  return NS_OK;
}

NS_IMETHODIMP
AlternateSourceChannel::GetContentDisposition(uint32_t* aContentDisposition)
{
  return mWrappedChannel->GetContentDisposition(aContentDisposition);
}

NS_IMETHODIMP
AlternateSourceChannel::SetContentDisposition(uint32_t aContentDisposition)
{
  return mWrappedChannel->SetContentDisposition(aContentDisposition);
}

NS_IMETHODIMP
AlternateSourceChannel::GetContentDispositionFilename(nsAString& aFilename)
{
  return mWrappedChannel->GetContentDispositionFilename(aFilename);
}

NS_IMETHODIMP
AlternateSourceChannel::SetContentDispositionFilename(const nsAString& aFilename)
{
  return mWrappedChannel->SetContentDispositionFilename(aFilename);
}

NS_IMETHODIMP
AlternateSourceChannel::GetContentDispositionHeader(nsACString& aHeader)
{
  return mWrappedChannel->GetContentDispositionHeader(aHeader);
}

NS_IMETHODIMP
AlternateSourceChannel::GetName(nsACString& aName)
{
  return mWrappedChannel->GetName(aName);
}

NS_IMETHODIMP
AlternateSourceChannel::IsPending(bool* aPending)
{
  return mWrappedChannel->IsPending(aPending);
}

NS_IMETHODIMP
AlternateSourceChannel::GetStatus(nsresult* aStatus)
{
  return mWrappedChannel->GetStatus(aStatus);
}

NS_IMETHODIMP
AlternateSourceChannel::Cancel(nsresult aStatus)
{
  mCanceled = true;
  mStatus = aStatus;
  return mWrappedChannel->Cancel(aStatus);
}

NS_IMETHODIMP
AlternateSourceChannel::Suspend()
{
  if (mPump) {
    mPump->Suspend();
  }
  return mWrappedChannel->Suspend();
}

NS_IMETHODIMP
AlternateSourceChannel::Resume()
{
  if (mPump) {
    mPump->Resume();
  }
  return mWrappedChannel->Resume();
}

NS_IMETHODIMP
AlternateSourceChannel::GetLoadGroup(nsILoadGroup** aLoadGroup)
{
  return mWrappedChannel->GetLoadGroup(aLoadGroup);
}

NS_IMETHODIMP
AlternateSourceChannel::SetLoadGroup(nsILoadGroup* aLoadGroup)
{
  return mWrappedChannel->SetLoadGroup(aLoadGroup);
}

NS_IMETHODIMP
AlternateSourceChannel::GetLoadFlags(nsLoadFlags* aLoadFlags)
{
  return mWrappedChannel->GetLoadFlags(aLoadFlags);
}

NS_IMETHODIMP
AlternateSourceChannel::SetLoadFlags(nsLoadFlags aLoadFlags)
{
  return mWrappedChannel->SetLoadFlags(aLoadFlags);
}

NS_IMETHODIMP
AlternateSourceChannel::OnStartRequest(nsIRequest* aRequest, nsISupports* aContext)
{
  MOZ_ASSERT(!mForwardToWrapped);

  nsresult rv = NS_OK;
    nsCOMPtr<nsIStreamListener> listener = do_QueryInterface(mWrappedChannel);
  if (listener) {
    rv = listener->OnStartRequest(this, mContext);
  }
  return rv;
}

NS_IMETHODIMP
AlternateSourceChannel::OnDataAvailable(nsIRequest* aRequest, nsISupports* aContext,
                                        nsIInputStream *aInputStream, uint64_t aOffset,
                                        uint32_t aCount)
{
  MOZ_ASSERT(!mCanceled);
  MOZ_ASSERT(!mForwardToWrapped);

  nsresult rv = NS_OK;
    nsCOMPtr<nsIStreamListener> listener = do_QueryInterface(mWrappedChannel);
  if (listener) {
    rv = listener->OnDataAvailable(this, mContext, aInputStream, aOffset, aCount);
  }
  return rv;
}

NS_IMETHODIMP
AlternateSourceChannel::OnStopRequest(nsIRequest* aRequest,
                                      nsISupports* aContext,
                                      nsresult aStatusCode)
{
  MOZ_ASSERT(!mForwardToWrapped);

  mStatus = aStatusCode;
  if (!mCanceled && NS_FAILED(mStatus)) {
    mCanceled = true;
  }
  mPending = false;

  nsresult rv = NS_OK;
    nsCOMPtr<nsIStreamListener> listener = do_QueryInterface(mWrappedChannel);
  if (listener) {
    rv = listener->OnStopRequest(this, mContext, aStatusCode);
  }

  mListener = nullptr;
  mContext = nullptr;
  mBody = nullptr;
  mPump = nullptr;
  return rv;
}

NS_IMETHODIMP
AlternateSourceChannel::OnNetworklessChannelReady()
{
  if (mCallback) {
    nsresult rv = mCallback->OnWrappedChannelReady(this);
    NS_ENSURE_SUCCESS(rv, rv);
  }
  return NS_OK;
}

NS_IMETHODIMP
AlternateSourceChannel::GetLoadInfo(nsILoadInfo** aLoadInfo)
{
  return mWrappedChannel->GetLoadInfo(aLoadInfo);
}

NS_IMETHODIMP
AlternateSourceChannel::SetLoadInfo(nsILoadInfo* aLoadInfo)
{
  return mWrappedChannel->SetLoadInfo(aLoadInfo);
}

} // namespace net
} // namespace mozilla
