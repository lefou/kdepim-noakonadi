
#include "ksync_return.h"

using namespace KitchenSync;

SyncReturn::SyncReturn()
{
    m_empty = true;
}
SyncReturn::SyncReturn( const QPtrList<KSyncEntry>& synced,
                        const QPtrList<KSyncEntry>& in1,
                        const QPtrList<KSyncEntry>& in2 )
{
    m_empty = false;
    m_sycned = synced;
    m_1not = in1;
    m_2not = in2;
}
SyncReturn::SyncReturn( const SyncReturn& newR)
{
    (*this) = newR;
}
SyncReturn::~SyncReturn()
{

}
SyncReturn &SyncReturn::operator=( const SyncReturn& ret )
{
    m_sycned = ret.m_sycned;
    m_1not = ret.m_1not;
    m_2not = ret.m_2not;
    m_empty = ret.m_empty;
    return *this;
}
QPtrList<KSyncEntry> SyncReturn::synced()
{
    return m_sycned;
}
QPtrList<KSyncEntry> SyncReturn::in1NotSynced()
{
    return  m_1not;
}
QPtrList<KSyncEntry> SyncReturn::in2NotSynced()
{
    return m_2not;
}
bool SyncReturn::isEmpty() const
{
    return m_empty;
}
