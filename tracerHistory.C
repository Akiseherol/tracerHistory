#include "tracerHistory.H"
#include "Cloud.H"
#include "dictionary.H"
#include "fvMesh.H"
#include "pointList.H"
#include "OFstream.H"
#include "addToRunTimeSelectionTable.H"

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(tracerHistory, 0);
    addToRunTimeSelectionTable(functionObject, tracerHistory, dictionary);
}
}


// * * * * * Constructors * * * * * //

Foam::functionObjects::tracerHistory::tracerHistory
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    fvMeshFunctionObject(name, runTime, dict),
    printf_(),
    precision_(IOstream::defaultPrecision()),
    applyFilter_(false),
    selectClouds_(),
    fieldName_(),
    directory_()
{
    read(dict);
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //
bool Foam::functionObjects::tracerHistory::read(const dictionary& dict)
{
    fvMeshFunctionObject::read(dict);

    printf_ = "";

    precision_ =
        dict.getOrDefault("precision", IOstream::defaultPrecision());

    selectClouds_.clear();
    dict.readIfPresent("clouds", selectClouds_);
    selectClouds_.uniq();

    if (selectClouds_.empty())
    {
        word cloudName;
        if (dict.readIfPresent("cloud", cloudName))
        {
            selectClouds_.push_back(std::move(cloudName));
        }
    }

    dict.readEntry("field", fieldName_);
    parcelSelect_ = dict.subOrEmptyDict("selection");

    directory_.clear();
    dict.readIfPresent("directory", directory_);

    if (directory_.size())
    {
        directory_.expand();
        if (!directory_.isAbsolute())
        {
            directory_ = time_.globalPath()/directory_;
        }
    }
    else
    {
        directory_ = time_.globalPath()
            /functionObject::outputPrefix/name();
    }

    directory_.clean();

    return true;
}

bool Foam::functionObjects::tracerHistory::execute()
{
    return true;
}


// * * * * * Write function — per-particle output * * * * * //

bool Foam::functionObjects::tracerHistory::write()
{
    const wordList cloudNames
    (
        selectClouds_.empty()
      ? mesh_.sortedNames<cloud>()
      : mesh_.sortedNames<cloud>(selectClouds_)
    );

    if (cloudNames.empty())
        return true;

    Log << name() << " output Time: " << time_.timeName() << nl;

    for (const word& cloudName : cloudNames)
    {
        // Find cloud
        const cloud* cloudPtr = mesh_.findObject<cloud>(cloudName);

        if (!cloudPtr)
        {
            WarningInFunction
                << "Cannot find cloud " << cloudName << endl;
            continue;
        }

        const cloud& currCloud = *cloudPtr;

        // Temporary registry 
        objectRegistry obrTmp
        (
            IOobject
            (
                "tmp::tracerHistory::" + cloudName,
                mesh_.time().constant(),
                mesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE,
                IOobject::NO_REGISTER
            )
        );

        // Populate with cloud fields 
        currCloud.writeObjects(obrTmp);

        // Retrieve particle positions
        const pointField* pointsPtr = cloud::findIOPosition(obrTmp);

        if (!pointsPtr)
        {
            WarningInFunction
                << "Cannot find positions for cloud " << cloudName << endl;
            continue;
        }

        const label nParcels = pointsPtr->size();

        const fileName outputDir = directory_ / cloudName;

        Log << "    cloud: " << cloudName
            << "  parcels: " << nParcels << endl;

        // Create directory 
	mkDir(outputDir);

	for (label i = 0; i < nParcels; ++i)
	{
	    const fileName file =
		outputDir / ("p" + Foam::name(i) + "_" + fieldName_ + ".dat");

	    writeParcel(file, cloudName, i);
	}

    }

    return true;
}


// * * * * * Write a single particle history * * * * * //

void Foam::functionObjects::tracerHistory::writeParcel
(
    const fileName& file,
    const word& cloudName,
    const label parcelI
) const
{
    const cloud* cloudPtr = mesh_.findObject<cloud>(cloudName);

    objectRegistry obrTmp
    (
        IOobject
        (
            "tmp::tracerHistory::" + cloudName,
            mesh_.time().constant(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE,
            IOobject::NO_REGISTER
        )
    );

    cloudPtr->writeObjects(obrTmp);

    
    const List<vector>* pos =
        cloud::findIOPosition(obrTmp);    

    const List<scalar>* fldS =
        obrTmp.findObject<IOField<scalar>>(fieldName_);

    const List<vector>* fldV =
        obrTmp.findObject<IOField<vector>>(fieldName_);

    // Appending mode
    OFstream os
    (
        file,
        IOstreamOption(IOstreamOption::ASCII, IOstreamOption::UNCOMPRESSED),
        IOstreamOption::APPEND
    );
    os.precision(precision_);

    // Time
    os << time_.value() << "  ";

    // Write particle positions
    if (pos)
    {
        const vector& p = (*pos)[parcelI];
        os << p.x() << " " << p.y() << " " << p.z() << "  ";
    }

    // Write fields
    if (fldS)
    {
        os << (*fldS)[parcelI] << nl;
    }
    else if (fldV)
    {
        const vector& v = (*fldV)[parcelI];
        os << v.x() << " " << v.y() << " " << v.z() << nl;
    }
    else
    {
        os << "0\n";
    }
}

