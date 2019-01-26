using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSIProject.DebugEngine
{
    public static class Win32Constants
    {
        ///<summary>
        ///Success code
        ///</summary>
        public const int S_OK = unchecked((int)0x00000000);
        ///<summary>
        ///Success code
        ///</summary>
        public const int NO_ERROR = unchecked((int)0x00000000);
        ///<summary>
        ///Success code
        ///</summary>
        public const int NOERROR = unchecked((int)0x00000000);

        ///<summary>
        ///Success code false
        ///</summary>
        public const int S_FALSE = unchecked((int)0x00000001);

        ///<summary>
        ///Catastrophic failure
        ///</summary>
        public const int E_UNEXPECTED = unchecked((int)0x8000FFFF);

        ///<summary>
        ///Not implemented
        ///</summary>
        public const int E_NOTIMPL = unchecked((int)0x80004001);

        ///<summary>
        ///Ran out of memory
        ///</summary>
        public const int E_OUTOFMEMORY = unchecked((int)0x8007000E);

        ///<summary>
        ///One or more arguments are invalid
        ///</summary>
        public const int E_INVALIDARG = unchecked((int)0x80070057);

        ///<summary>
        ///No such interface supported
        ///</summary>
        public const int E_NOINTERFACE = unchecked((int)0x80004002);

        ///<summary>
        ///Invalid pointer
        ///</summary>
        public const int E_POINTER = unchecked((int)0x80004003);

        ///<summary>
        ///Invalid handle
        ///</summary>
        public const int E_HANDLE = unchecked((int)0x80070006);

        ///<summary>
        ///Operation aborted
        ///</summary>
        public const int E_ABORT = unchecked((int)0x80004004);

        ///<summary>
        ///Unspecified error
        ///</summary>
        public const int E_FAIL = unchecked((int)0x80004005);

        ///<summary>
        ///General access denied error
        ///</summary>
        public const int E_ACCESSDENIED = unchecked((int)0x80070005);

        ///<summary>
        ///The data necessary to complete this operation is not yet available.
        ///</summary>
        public const int E_PENDING = unchecked((int)0x8000000A);
    }
}
