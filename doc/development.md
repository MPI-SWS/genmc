# Modifying GenMC

## Adding new label types

To add a new label, follow these steps:

1. Add a number and a name for the new label in Instruction.def
2. Add a class for the new label in ExecutionGraph/EventLabel.hpp.
   Macros are provided for dummy labels and standard subclasses
3. Add a case for the new label in LabelVisitor.hpp
4. Define how the label should be printed in EventLabel.cpp
   (optionally: LabelPrinterBase too)
5. Create a handler for the new label in GenMCDriver.{hpp,cpp}
   (if necessary), and at DriverHandlerDispatcher
6. In case a new LLVM-IR function leads to the creation of
   the new label:
   - Add a number and a name for the internal function in
   Runtime/InternalFunction.def.
   - Define a function that dispatches the driver at
   Runtime/Execution.cpp

*Note:* the procedure above describes the bare minimum, and works well
for dummy labels or subclasses of existing labels. If e.g., the new
label has attributes like location then extra changes might be
required to ensure that iterators, etc still work.
