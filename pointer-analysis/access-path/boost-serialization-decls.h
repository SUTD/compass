#ifndef _BOOST_SERIALIZATION_DECL_
#define _BOOST_SERIALIZATION_DECL_

#include "access-path.h"
#include "term.h"
#include "cnode.h"
#include "il/type.h"
#include "sail/Variable.h"
#include "Constraint.h"
#include "MemoryLocation.h"
#include "Edge.h"
#include "Error.h"
#include "error-traces.h"
#include "SummaryGraph.h"
#include "AnalysisResult.h"
#include "call_id.h"
#include "../MemoryAnalysis.h"
#include "Identifier.h"
#include "FileIdentifier.h"
#include "ProjectSettings.h"
#include "SummaryStream.h"




BOOST_CLASS_EXPORT(Term);
BOOST_CLASS_EXPORT(VariableTerm);
BOOST_CLASS_EXPORT(ConstantTerm);
BOOST_CLASS_EXPORT(FunctionTerm);
BOOST_CLASS_EXPORT(ArithmeticTerm);

BOOST_CLASS_EXPORT(BooleanVar);
BOOST_CLASS_EXPORT(CNode);
BOOST_CLASS_EXPORT(Connective);
BOOST_CLASS_EXPORT(EqLeaf);
BOOST_CLASS_EXPORT(False);
BOOST_CLASS_EXPORT(True);
BOOST_CLASS_EXPORT(ILPLeaf);
BOOST_CLASS_EXPORT(Leaf);
BOOST_CLASS_EXPORT(ModLeaf);
BOOST_CLASS_EXPORT(QuantifiedLeaf);


BOOST_CLASS_EXPORT(offset);
BOOST_CLASS_EXPORT(AccessPath);
BOOST_CLASS_EXPORT(Address);
BOOST_CLASS_EXPORT(info_item);
BOOST_CLASS_EXPORT(alloc_info);
BOOST_CLASS_EXPORT(Alloc);
BOOST_CLASS_EXPORT(ArithmeticValue);
BOOST_CLASS_EXPORT(ArrayRef);
BOOST_CLASS_EXPORT(ConstantValue);
BOOST_CLASS_EXPORT(Deref);
BOOST_CLASS_EXPORT(FieldSelection);
BOOST_CLASS_EXPORT(FunctionValue);
BOOST_CLASS_EXPORT(IndexVariable);
BOOST_CLASS_EXPORT(IterationCounter);
BOOST_CLASS_EXPORT(UnmodeledValue);
BOOST_CLASS_EXPORT(Variable);
BOOST_CLASS_EXPORT(StringLiteral);
BOOST_CLASS_EXPORT(AbstractDataStructure);
BOOST_CLASS_EXPORT(Nil);
BOOST_CLASS_EXPORT(ProgramFunction);
BOOST_CLASS_EXPORT(TypeConstant);
BOOST_CLASS_EXPORT(call_id);



BOOST_CLASS_EXPORT(Constraint);
BOOST_CLASS_EXPORT(MemoryLocation);
BOOST_CLASS_EXPORT(Edge);
BOOST_CLASS_EXPORT(ErrorTrace);
BOOST_CLASS_EXPORT(AssertionTrace);
BOOST_CLASS_EXPORT(ReadTrace);
BOOST_CLASS_EXPORT(DerefTrace);
BOOST_CLASS_EXPORT(BufferTrace);
BOOST_CLASS_EXPORT(CastErrorTrace);
BOOST_CLASS_EXPORT(DoubleDeleteTrace);
BOOST_CLASS_EXPORT(DeletedAccessTrace);
BOOST_CLASS_EXPORT(MemoryLeakTrace);

BOOST_CLASS_EXPORT(callstack_entry);
BOOST_CLASS_EXPORT(access_info);
BOOST_CLASS_EXPORT(SummaryGraph);
BOOST_CLASS_EXPORT(SummaryStream);

BOOST_CLASS_EXPORT(Error);
BOOST_CLASS_EXPORT(AnalysisResult);
BOOST_CLASS_EXPORT(dotty_info);


BOOST_CLASS_EXPORT(FileIdentifier);
BOOST_CLASS_EXPORT(Identifier);
BOOST_CLASS_EXPORT(ProjectSettings);




BOOST_CLASS_IMPLEMENTATION(SummaryGraph, boost::serialization::object_serializable);



#endif /* _BOOST_SERIALIZATION_DECL_ */
