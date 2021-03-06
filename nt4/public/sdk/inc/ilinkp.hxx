//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       ilinkp.hxx
//
//  Contents:   Declarations PRIVATE to link tracking code across
//              multiple Cairo projects.
//              Do not use these declarations without first contacting
//              the current owner of this file.
//
//  Classes:    CTracker              -- base for all tracking links
//              COleTrackingLink      -- base for shell and ole links
//              COleLinkTrackingLink  -- tracking for IOleLink def link
//              COleShellTrackingLink -- tracking for shell ref
//
//  Functions:
//
//  History:    07-Aug-93   BillMo      Created.
//
//  Note:       Id's of source are retrieved only when the source is bound to.
//
//--------------------------------------------------------------------------

#ifndef __ILINKP_HXX__
#define __ILINKP_HXX__

#include <lnkobjid.hxx>

#define ELEMENTS(x) (sizeof(x)/sizeof((x)[0]))
#define WCHARS(x) ELEMENTS(x)
#define ALIGN8(cb) (((cb-1) & (~7)) + 8)

#define WCH_COMP_SEPARATOR L'\\'
#define WCH_ROOT_SEPARATOR L':'

#define USER_REGISTRY_ROOT L"SoftWare\\Microsoft\\LinkSearch"
#define ABSOLUTE_MAX_AUTO    64      // volumes in auto searchlist
#define DEFAULT_MAX_AUTO     10
#define MAX_BROADCASTS       10
#define BROADCAST_REPLY_WAIT 300

#define DEFAULT_GROVEL_MONIKER_SIZE 1024

// new (ExceptOnFail) SFindObjectOut [FSCTL_OUT_BUFFER_SIZE];
#define FSCTL_OUT_BUFFER_SIZE 10

//
// treepatcher won't allow paths > MAX_DOS_NETWORK_PATH
//
#define MAX_DOS_NETWORK_PATH 384
#define MAX_NT_HEADER        20
#define MAX_NT_NETWORK_PATH  (MAX_DOS_NETWORK_PATH + MAX_NT_HEADER)

#define MAX_EMBEDDINGLEVELS  40

#define TREECOPY_PAGESIZE 4096
#define TREEPATCHER_SIG LONGSIG('h','c','p','y')



class CVolume;
class CSearchContext;
class CEmbeddedMonikers;

//--------------------------------------------------------------------------
//
// Currently private definitions that may become public.
//
//--------------------------------------------------------------------------

typedef DWORD HCOPY;

//--------------------------------------------------------------------------
//
// Private error codes.
//
//--------------------------------------------------------------------------

#if !defined(LNK_NO_OLE2)

//
// Error codes (HRESULT)0x80081580 to 0x800815bf are internal
// to link tracking.
//

//
// Generated by CVolume::FindObject to indicate the volume was
// not even searched.
//
#define LINKP_E_VOLUME_NOT_SEARCHED  ((HRESULT)0x80081580)

//
// Generated by objectid.cxx::ReadObjectId(IPropertyStorage*,OBJECTID*)
// as an internal error to GetObjectId.
//
#define LINKP_E_NOSUCHPROPERTY       ((HRESULT)0x80081581)

//
// Generated by CVolume::FindObject to indicate that the volume was
// successfully contacted, but no object with matching id was found.
//
#define LINKP_E_VOLUME_SEARCHED_OBJECT_NOT_FOUND ((HRESULT)0x80081582)

//
// Used in failure testing
//
#define LINKP_E_FAILTEST ((HRESULT)0x80081583)

//
// Returned by ReplaceFileMoniker if there is no file moniker to replace.
//
#define LINKP_E_NOFILEMONIKER ((HRESULT)0x80081584)

//
// IEnumMoniker::Next returned S_FALSE immediately after IMoniker::Enum
// was called successfully.
//
#define LINKP_E_NOMONIKERS    ((HRESULT)0x80081585)

//
// Couldn't rebuild moniker to pseudo-object because there
// weren't enough monikers in the enumeration.
//
#define LINKP_E_RANGE_INACCESSIBLE ((HRESULT) 0x80081586)

//
// The path passed to CPathWalker was too long (i.e. absolute moniker
// path name too long.)
//
#define LINKP_E_INVALID_PATH ((HRESULT) 0x80081587)

//
// The ancestor was not found by CPathWalker
//
#define LINKP_E_ANCESTOR_NOT_FOUND ((HRESULT) 0x80081588)

//
// No object which is the only object with matching lineage and
// last component of name
//
#define LINKP_E_NOLINEAGEMATCH     ((HRESULT) 0x80081589)

//
// No object exactly matches the id
//
#define LINKP_E_NOEXACTMATCH       ((HRESULT) 0x80081589)

//
// No monikers in IMoniker::Enum
//
#define LINKP_E_ENUMEMPTY          ((HRESULT) 0x8008158a)

//
// Too many nesting levels
//
#define LINKP_E_TOOMANYEMBEDDINGLEVELS ((HRESULT) 0x8008158b)

//
// Used when throwing exceptions due to not enough memory
//
#define LINK_E_NOT_ENOUGH_MEMORY \
    (HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY))

#endif // !defined(LNK_NO_OLE2)

//--------------------------------------------------------------------------
//
// Defines for tracker object serialized form
//
//--------------------------------------------------------------------------

#define SHELLMK_SIG        0x0033
#define OLEMK_SIG          0x0022
#define MAX_LINK_EXPANSION 262144

#define LINK_FLAG_DOMAINID 0x00000001
#define LINK_FLAG_VOLUMEID 0x00000002
#define LINK_FLAG_OBJECTID 0x00000004

//--------------------------------------------------------------------------
//
// Function predeclarations
//
//--------------------------------------------------------------------------

void UpdateAutoSearchList(void);

//--------------------------------------------------------------------------
//
// Functions used by DRT.
//
//--------------------------------------------------------------------------

const WCHAR *ObjectIdToString(const OBJECTID &oid);

//+-------------------------------------------------------------------------
//
//  Class:      DFSID
//
//  Purpose:    Encapsulate specifics of DFS domain and volume ids.
//
//--------------------------------------------------------------------------

class DFSID
{
public:
    DFSID()
    {
        Invalidate();
    }
    VOID Invalidate(VOID)
    {
        memset(&g, 0, sizeof(g));
    }
    operator GUID ()
    {
        return(g);
    }
    operator GUID * ()
    {
        return(&g);
    }
    operator == (const DFSID &other)
    {
        return(0 == memcmp(&g, &other.g, sizeof(g)));
    }
    BOOL IsValid(VOID)
    {
        DFSID i;

        return(i != *this);
    }
private:
    GUID g;
};

typedef DFSID DOMAINID;
typedef DFSID VOLUMEID;

//+-------------------------------------------------------------------------
//
//  Class:      CTracker
//
//  Purpose:    Base class for 1. extending Ole links, 2. supporting
//              shell references, 3. supporting win32 links.
//
//              This class contains code common to all types.
//
//  Interface:
//
//  History:    07-Aug-93   BillMo      Created.
//
//  Notes:
//
//--------------------------------------------------------------------------

#if !defined(LNK_NO_OLE2)

// this should be put in its own header
class CTracker
{
public:

            CTracker();

            // assignment operators/ctors needed because of pointer.

            // doesn't copy expansion data
            CTracker(const CTracker &);

            // doesn't overwrite expansion data
            CTracker & CTracker::operator = (const CTracker & t);

            ~CTracker();


    //
    // For OLE2 tracking
    //
    BOOL    operator == (const CTracker &t);
    BOOL    operator != (const CTracker &t);

    HRESULT Load(IStorage *pstg);
    HRESULT Save(IStorage *pstg);
    HRESULT BindToObject(IBindCtx *pbc,
                         IMoniker *pmkToLeft,
                         REFIID    riid,
                         void **   ppv,
                         IMoniker**ppmk);
    HRESULT UpdateIdsFromMoniker(IBindCtx *pbc, IMoniker *pmk);

    //
    // For OLE1 tracking
    //
    VOID    SetObjectId(const OBJECTID &oid);

    //
    // For OLE1 and OLE2 tracking
    //
    HRESULT FindObjectName(CVolume * pVolume,
                           BOOL fHintedVolumeValid,
                           WCHAR ** ppwszPath,
                           CEmbeddedMonikers *pem,
                           CSearchContext *psc,
                           const WCHAR *pwszOriginal);

private:

    HRESULT SaveTrackingInfo(USHORT usSig, LPSTREAM pstm);
    void    Read(LPSTREAM pstm, void *pv, ULONG cbExpected);
    HRESULT LoadTrackingInfo(USHORT usSig, LPSTREAM pstm);
    HRESULT SearchEnumVolumes(CVolume *pVolume,
                              WCHAR **ppwszPath,
                              CEmbeddedMonikers *pem,
                              CSearchContext *psc,
                              const WCHAR *pwszOriginal);
    HRESULT GetEmbeddedObjectsId(IBindCtx *pbc, IMoniker *pmk);
    HRESULT wBindToObject(CSearchContext *psc,
                         IMoniker *pmkToLeft,
                         REFIID    riid,
                         void **   ppv,
                         IMoniker**ppmk);

    HRESULT GetVolume(IBindCtx * pbc, IMoniker *pmkFile, CVolume *pVolume);


    //
    // if persistent data is added, then GetSizeMax must be
    // changed.
    //
    ULONG       _ulFlags;

    DOMAINID    _didDomain;
    VOLUMEID    _vidVolume;
    OBJECTID    _oidFile;
    OBJECTID    _oidEmbed;
    LONG        _cNoStorage;

    // other cairo info

    // expansion data
    ULONG       _cbExpansion;
    BYTE *      _pbExpansion;

};
#endif // !defined(LNK_NO_OLE2)

#if !defined(LNK_NO_NTDEF)
NTSTATUS PatchObjectId(UNICODE_STRING *pus, const OBJECTID &oid);
#endif

// BUGBUG this class definition should be in its own header file
//        when we get rid of lnkfsctl.exe's dependence on lnktrack.dll

#if !defined(LNK_NO_EXCEPTIONS)
//+-------------------------------------------------------------------------
//
//  Class:  CFileHandle
//
//  Purpose:    Handle opening files for caller if necessary.
//
//  Interface:  CFileHandle::CFileHandle  -- Initialize.
//              CFileHandle::Open         -- Open path.
//              CFileHandle::OpenIf       -- Open path if handle is not passed.
//              CFileHandle::~CFileHandle -- Close file if we opened it.
//
//  History:    07-Jun-92   BillMo      Created.
//
//  Notes:      BUGBUG: should probably have a single unwindable object
//                      which contains all these resources.
//
//--------------------------------------------------------------------------

#include <except.hxx>

class CFileHandle
{
public:
            CFileHandle();
            ~CFileHandle();

    NTSTATUS OpenIf(HANDLE           hFile,
                    const WCHAR *    pwszPath,
                    DWORD            dwAccess,
                    DWORD            dwShare);

    NTSTATUS Open(const WCHAR *  pwszDosPath,
                  ACCESS_MASK    AccessMask = GENERIC_READ | SYNCHRONIZE,
                  ULONG          ShareAccess = FILE_SHARE_READ);

    inline  operator HANDLE ();

private:

    HANDLE  _hFile;
    BOOL    _fOpened;
};

//+-------------------------------------------------------------------
//
//  Member:     CFileHandle::operator HANDLE, public
//
//  Synopsis:   Return internal handle.  For HANDLE parameter passing.
//
//  History:    25-Jan-93 BillMo    Created.
//
//  Notes:
//
//--------------------------------------------------------------------

CFileHandle::operator HANDLE (void)
{
    return(_hFile);
}
#endif // !defined(LNK_NO_EXCEPTIONS)

#if !defined(LNK_NO_NTDEF)
VOID PrependNtObjectName(UNICODE_STRING *pus);
#endif

#endif

