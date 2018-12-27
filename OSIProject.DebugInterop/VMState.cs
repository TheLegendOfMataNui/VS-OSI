using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSIProject.DebugInterop
{
    public class VMState
    {
        public enum VMExecutionState : byte
        {
            Unknown = 0,
            NativeCode = 1,
            OSIRunning = 2,
            OSISuspended = 3,
        }

        public event EventHandler ExecutionStateChanged;

        public VMExecutionState ExecutionState { get; private set; }
        private uint _instructionPointer = 0;
        public uint InstructionPointer
        {
            get
            {
                //if (ExecutionState != VMExecutionState.OSISuspended)
                    //throw new VMStateException("", VMExecutionState.OSISuspended);
                return _instructionPointer;
            }
        }

        public void OnExecutionStateChanged(object sender, ServerStateChangePayload change)
        {
            ExecutionState = change.ExecutionState;
            _instructionPointer = change.InstructionPointer;
            ExecutionStateChanged?.Invoke(this, new EventArgs());
        }
    }

    public class VMStateException : InvalidOperationException
    {
        public VMState.VMExecutionState CorrectState { get; }

        public VMStateException(string message, VMState.VMExecutionState correctState) : base(message)
        {
            this.CorrectState = correctState;
        }
    }
}
