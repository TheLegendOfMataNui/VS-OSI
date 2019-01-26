using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Microsoft.VisualStudio.Debugger.Interop;

namespace OSIProject.DebugEngine
{
    /*public class AD7Engine : IDebugEngine2, IDebugEngineLaunch2, IDebugEngineProgram2
    {
        private OSIProject.DebugInterop.DebugConnection Connection = null;

        #region IDebugEngine2 Members
        public int EnumPrograms(out IEnumDebugPrograms2 ppEnum)
        {
            throw new NotImplementedException();
        }

        public int Attach(IDebugProgram2[] rgpPrograms, IDebugProgramNode2[] rgpProgramNodes, uint celtPrograms, IDebugEventCallback2 pCallback, enum_ATTACH_REASON dwReason)
        {
            throw new NotImplementedException();
        }

        public int CreatePendingBreakpoint(IDebugBreakpointRequest2 pBPRequest, out IDebugPendingBreakpoint2 ppPendingBP)
        {
            throw new NotImplementedException();
        }

        public int SetException(EXCEPTION_INFO[] pException)
        {
            throw new NotImplementedException();
        }

        public int RemoveSetException(EXCEPTION_INFO[] pException)
        {
            throw new NotImplementedException();
        }

        public int RemoveAllSetExceptions(ref Guid guidType)
        {
            throw new NotImplementedException();
        }

        public int GetEngineId(out Guid pguidEngine)
        {
            pguidEngine = new Guid(EngineConstants.EngineGUID);
            return Win32Constants.S_OK;
        }

        public int DestroyProgram(IDebugProgram2 pProgram)
        {
            throw new NotImplementedException();
        }

        public int ContinueFromSynchronousEvent(IDebugEvent2 pEvent)
        {
            throw new NotImplementedException();
        }

        public int SetLocale(ushort wLangID)
        {
            // Not implemented
            return Win32Constants.S_OK;
        }

        public int SetRegistryRoot(string pszRegistryRoot)
        {
            // Not implemented
            return Win32Constants.S_OK;
        }

        public int SetMetric(string pszMetric, object varValue)
        {
            // Not implemented
            return Win32Constants.S_OK;
        }

        public int CauseBreak()
        {
            Connection.Break();
            return Win32Constants.S_OK;
        }
        #endregion
    }*/
}
