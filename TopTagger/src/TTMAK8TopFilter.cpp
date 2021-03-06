#include "TopTagger/TopTagger/interface/TTMAK8TopFilter.h"

#include "TopTagger/TopTagger/interface/TopTaggerResults.h"
#include "TopTagger/CfgParser/include/Context.hh"
#include "TopTagger/CfgParser/include/CfgDocument.hh"

void TTMAK8TopFilter::getParameters(const cfg::CfgDocument* cfgDoc, const std::string& localContextName)
{
    //Construct contexts
    cfg::Context commonCxt("Common");
    cfg::Context localCxt(localContextName);
    
    //Parameters
    type_        = static_cast<TopObject::Type>(cfgDoc->get("type",        localCxt,  TopObject::MERGED_TOP));

}

void TTMAK8TopFilter::run(TopTaggerResults& ttResults)
{
    //Get the list of top candidates as generated by the clustering algo
    std::vector<TopObject>& topCandidates = ttResults.getTopCandidates();
    //Get the list of final tops into which we will stick candidates
    std::vector<TopObject*>& tops = ttResults.getTops();

    //This class adds the merged objects to the final top list 
    for(auto& topCand : topCandidates)
    {
        //For now this just adds the merged tops
        if(topCand.getType() == type_)
        {
            tops.push_back(&topCand);
        }
    }    
}
