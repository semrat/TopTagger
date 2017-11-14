#include "TopTagger/TopTagger/include/TTMTMVA.h"

#include "TopTagger/TopTagger/include/TopTaggerUtilities.h"
#include "TopTagger/TopTagger/include/TopObject.h"
#include "TopTagger/TopTagger/include/TopTaggerResults.h"
#include "TopTagger/CfgParser/include/Context.hh"
#include "TopTagger/CfgParser/include/CfgDocument.hh"
#include "TopTagger/CfgParser/include/TTException.h"

void TTMTMVA::getParameters(const cfg::CfgDocument* cfgDoc, const std::string& localContextName)
{
#ifdef SHOTTOPTAGGER_DO_TMVA
    //Construct contexts
    cfg::Context commonCxt("Common");
    cfg::Context localCxt(localContextName);

    discriminator_ = cfgDoc->get("discCut",       localCxt, -999.9);
    modelFile_     = cfgDoc->get("modelFile",     localCxt, "");
    modelName_     = cfgDoc->get("modelName",     localCxt, "");
    NConstituents_ = cfgDoc->get("NConstituents", localCxt, 3);

    int iVar = 0;
    bool keepLooping;
    do
    {
        keepLooping = false;

        //Get variable name
        std::string varName = cfgDoc->get("mvaVar", iVar, localCxt, "");

        //if it is a non empty string save in vector
        if(varName.size() > 0)
        {
            keepLooping = true;

            vars_.push_back(varName);
        }
        ++iVar;
    }
    while(keepLooping);

    //create TMVA reader
    reader_ = new TMVA::Reader( "!Color:!Silent" );
    if(reader_ == nullptr)
    {
        //Throw if this is an invalid pointer
        THROW_TTEXCEPTION("TMVA reader creation failed!!!");
    }

    //load model file into reader
    auto* imethod = reader_->BookMVA( modelName_.c_str(), modelFile_.c_str() );
    if(imethod == nullptr)
    {
        //Throw if this is an invalid pointer
        THROW_TTEXCEPTION("TMVA reader could not load model named \"" + modelName_ + "\" from file \"" + modelFile_ + "\"!!!");        
    }

    //load variables into reader
    for(const auto& var : vars_)
    {
        varMap_[var]=0;
        reader_->AddVariable(var.c_str(), &varMap_[var]);
    }

#else
    THROW_TTEXCEPTION("Top tagger was not compiled with support for TMVA!!!!"); 
#endif
}

void TTMTMVA::run(TopTaggerResults& ttResults)
{
#ifdef SHOTTOPTAGGER_DO_TMVA
    //Get the list of top candidates as generated by the clustering algo
    std::vector<TopObject>& topCandidates = ttResults.getTopCandidates();
    //Get the list of final tops into which we will stick candidates
    std::vector<TopObject*>& tops = ttResults.getTops();

    for(auto& topCand : topCandidates)
    {
        //We only want to apply the MVA algorithm to triplet tops
        if(topCand.getNConstituents() == NConstituents_)
        {
            //Prepare data from top candidate (this code is shared with training tuple producer)
            //Perhaps one day the intermediate map can be bypassed ...
            std::map<std::string, double> varMap = ttUtility::createMVAInputs(topCand, 0.8);

            //Set the values in the reader (as usual, this is inefficient, rework createMVAInputs)
            for(const auto& var : vars_)
            {
                varMap_[var] = varMap[var];
            }

            //predict value
            double discriminator = reader_->EvaluateMVA(modelName_);
            topCand.setDiscriminator(discriminator);

            //place in final top list if it passes the threshold
            if(discriminator > discriminator_)
            {
                tops.push_back(&topCand);
            }
        }
    }
#endif
}
