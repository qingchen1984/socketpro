#ifndef	___UCOMM__UTIL_HEADER_FILE___H___
#define ___UCOMM__UTIL_HEADER_FILE___H___

#include <assert.h>
#include "definebase.h"
#include <sstream>

#ifdef WIN32_64	
#include "wincommutil.h"
#else
#include "nixcommutil.h"
#include "bvariant.h"
#endif

#define CUExCode(errMsg, errCode)  SPA::CUException(errMsg, __FILE__, __LINE__, __FUNCTION__, errCode)
#define CUEx(errMsg)  CUExCode(errMsg, MB_ERROR_UNKNOWN)
#define CUSEx(errMsg) CUExCode(errMsg, MB_BAD_DESERIALIZATION)

namespace SPA {

    typedef CComVariant UVariant;

    static void ParseDec(const char *data, DECIMAL &dec) {
        assert(data);
        dec.Hi32 = 0;
        dec.wReserved = 0;
        const char* posNegative = strchr(data, '-');
        if (posNegative) {
            dec.sign = 0x80;
            ++data;
        } else {
            dec.sign = 0;
        }
        const char *end;
        UINT64 temp = atoull(data, end);
        if (*end == '.') {
            const char *start = ++end;
            UINT64 dot = atoull(end, end);
            unsigned char scale = (unsigned char) (end - start);
            dec.scale = scale;
            while (scale) {
                temp = (temp << 3) + (temp << 1);
                --scale;
            }
            dec.Lo64 = temp + dot;
        } else {
            dec.Lo64 = temp;
            dec.scale = 0;
        }
    }

    static inline double ToDouble(const DECIMAL &dec) {
        unsigned char scale = dec.scale;
        double d = (double) dec.Lo64;
        UINT64 temp = 1;
        while (scale) {
            temp = (temp << 3) + (temp << 1);
            --scale;
        }
        d /= temp;
        if (dec.sign) {
            d = -d;
        }
        return d;
    }

    static bool IsEqual(const tagVARIANT &vt0, const tagVARIANT &vt1) {
        if (vt0.vt == VT_EMPTY && vt1.vt == VT_EMPTY) {
            return true;
        } else if (vt0.vt == VT_NULL && vt1.vt == VT_NULL) {
            return true;
        }

        VARTYPE vtMe = vt0.vt;
        VARTYPE vtSrc = vt1.vt;
        if (vtMe != vtSrc) {
            if (vtMe == VT_INT) {
                vtMe = VT_I4;
            } else if (vtMe == VT_UINT) {
                vtMe = VT_UI4;
            } else if (vtMe == (VT_INT | VT_ARRAY)) {
                vtMe = (VT_I4 | VT_ARRAY);
            } else if (vtMe == (VT_UINT | VT_ARRAY)) {
                vtMe = (VT_UI4 | VT_ARRAY);
            }

            if (vtSrc == VT_INT) {
                vtSrc = VT_I4;
            } else if (vtSrc == VT_UINT) {
                vtSrc = VT_UI4;
            } else if (vtSrc == (VT_INT | VT_ARRAY)) {
                vtSrc = (VT_I4 | VT_ARRAY);
            } else if (vtSrc == (VT_UINT | VT_ARRAY)) {
                vtSrc = (VT_UI4 | VT_ARRAY);
            }
        }
        if (vtMe != vtSrc)
            return false;
        if ((vtMe & VT_ARRAY) == VT_ARRAY) {
            if (vt0.parray == vt1.parray)
                return true;
            if (!vt0.parray && vt1.parray)
                return false;
            if (vt0.parray && !vt1.parray)
                return false;
            unsigned int len = vt0.parray->rgsabound->cElements;
            unsigned int lenSrc = vt1.parray->rgsabound->cElements;
            if (len != lenSrc)
                return false;
            PVOID p, pSrc;
            bool equal = true, fixed = false;
            SafeArrayAccessData(vt1.parray, &pSrc);
            SafeArrayAccessData(vt0.parray, &p);
            for (unsigned int n = 0; n < len; ++n) {
                switch (vtMe & (~VT_ARRAY)) {
                    case VT_BSTR:
                        if (vt0.bstrVal && vt1.bstrVal) {
                            unsigned int chars = SysStringLen(vt0.bstrVal);
                            unsigned int charsSrc = SysStringLen(vt1.bstrVal);
                            if (chars == charsSrc) {
                                equal = (::memcmp(vt0.bstrVal, vt1.bstrVal, chars * sizeof (wchar_t)) == 0);
                            } else {
                                equal = false;
                            }
                        } else {
                            equal = (vt0.bstrVal == vt1.bstrVal);
                        }
                        break;
                    case VT_VARIANT:
                        equal = IsEqual(((const tagVARIANT*) p)[n], ((const tagVARIANT*) pSrc)[n]);
                        break;
                    default:
                        fixed = true;
                        break;
                }
                if (!equal || fixed)
                    break;
            }
            if (equal && fixed) {
                equal = (::memcmp(p, pSrc, len * vt0.parray->cbElements) == 0);
            }
            SafeArrayUnaccessData(vt0.parray);
            SafeArrayUnaccessData(vt1.parray);
            return equal;
        } else {
            switch (vtMe) {
                case VT_BSTR:
                {
                    if (vt0.bstrVal == vt1.bstrVal)
                        return true;
                    if (!vt0.bstrVal && vt1.bstrVal)
                        return false;
                    if (vt0.bstrVal && !vt1.bstrVal)
                        return false;
                    unsigned int len = SysStringLen(vt0.bstrVal);
                    unsigned int lenSrc = SysStringLen(vt1.bstrVal);
                    if (len != lenSrc)
                        return false;
                    return (::memcmp(vt0.bstrVal, vt1.bstrVal, len * sizeof (wchar_t)) == 0);
                }
                case VT_I1:
                case VT_UI1:
                    return (vt0.bVal == vt1.bVal);
                case VT_I2:
                case VT_UI2:
                    return (vt0.uiVal == vt1.uiVal);
                case VT_I4:
                case VT_UI4:
                case VT_INT:
                case VT_UINT:
                    return (vt0.ulVal == vt1.ulVal);
                case VT_I8:
                case VT_UI8:
#ifndef _WIN32_WCE
                    return (vt0.ullVal == vt1.ullVal);
#else
                    return (vt0.cyVal.int64 == vt1.cyVal.int64);
#endif
                case VT_R4:
                    return (vt0.fltVal == vt1.fltVal);
                case VT_R8:
                    return (vt0.dblVal == vt1.dblVal);
                case VT_BOOL:
                {
                    bool b0 = vt0.boolVal ? true : false;
                    bool b1 = vt1.boolVal ? true : false;
                    return (b0 == b1);
                }
                case VT_DATE:
#ifdef WIN32_64
                    return (vt0.date == vt1.date);
#else
                    return (vt0.ullVal == vt1.ullVal);
#endif
                case VT_CY:
                    return (vt0.cyVal.int64 == vt1.cyVal.int64);
                case VT_DECIMAL:
                case VT_CLSID:
                    return (::memcmp(&vt0.decVal, &vt1.decVal, sizeof (tagDEC)) == 0);
                default:
                    assert(false); //not implemented and shouldn't come here
                    break;
            }
        }
        return false;
    }

    /** 
     * Map a variant onto MS VARIANT data type
     * @param v A reference to a variant instance
     * @return MS VARIANT data type
     */
    inline static unsigned short Map2VarintType(const UVariant &v) {
        return v.vt;
    }
};

#endif
