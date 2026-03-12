/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           |
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Derived from dataCloud.H
\*---------------------------------------------------------------------------*/

#ifndef tracerHistory_H
#define tracerHistory_H

#include "fvMeshFunctionObject.H"
#include "parcelSelectionDetail.H"
#include "vectorField.H"

namespace Foam
{
namespace functionObjects
{

class tracerHistory
:
    public fvMeshFunctionObject,
    public Foam::Detail::parcelSelection
{
    // Private Data
    string printf_;
    unsigned precision_;
    bool applyFilter_;
    wordRes selectClouds_;
    word fieldName_;
    fileName directory_;

    // Templates retained for compatibility (not used for per-particle writing)
    template<class Type>
    static void writePointValue(Ostream& os, const vector& pt, const Type& val);

    template<class Type>
    static void writeList(Ostream& os, const vectorField& points, const List<Type>& field);

    template<class Type>
    static void writeListParallel(Ostream& os, const vectorField& points, const List<Type>& field);

    template<class Type>
    static void writeList(Ostream& os, const vectorField& points, const List<Type>& field, const bitSet& selected);

    template<class Type>
    static void writeListParallel(Ostream& os, const vectorField& points, const List<Type>& field, const bitSet& selected);

    // Unused legacy function
    bool writeCloud(const fileName&, const word&) { return false; }

    template<class Type>
    bool writeField(const fileName&, const objectRegistry&) const;

    tracerHistory(const tracerHistory&) = delete;
    void operator=(const tracerHistory&) = delete;

protected:

    // NEW — write a single particle’s history file
    void writeParcel
    (
        const fileName& file,
        const word& cloudName,
        const label parcelI
    ) const;

public:

    TypeName("tracerHistory");

    tracerHistory(const word&, const Time&, const dictionary&);
    virtual ~tracerHistory() = default;

    virtual bool read(const dictionary&);
    virtual bool execute();
    virtual bool write();
};

} // namespace functionObjects
} // namespace Foam

#ifdef NoRepository
 #include "tracerHistoryTemplates.C"
#endif

#endif

