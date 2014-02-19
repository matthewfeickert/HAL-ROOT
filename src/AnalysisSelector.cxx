#include <HAL/AnalysisSelector.h>

namespace HAL {

void AnalysisSelector::Init (TTree *tree) {
  // The Init() function is called when the selector needs to initialize
  // a new tree or chain.
  // Init() will be called many times when running on PROOF
  // (once per file to be processed).

  std::cout << "Processing file number: " << tree->GetFileNumber() << std::endl;
  
  // attach raw AnalysisData object to data map
  TMap *loopingData = new TMap();
  loopingData->SetName("loopingData");
  loopingData->Add(new TObjString("RawData"), new AnalysisData(fChain));

  // attach map to fInput
  fInput->AddFirst(loopingData);
  
  // attach data map to Algorithm(s)
  fAnalysisFlow->AssignData((TMap*)fInput->FindObject("loopingData"));
}

Bool_t AnalysisSelector::Notify () {
  // The Notify() function is called when a new file is opened. This
  // can be either for a new TTree in a TChain or when when a new TTree
  // is started when using PROOF. 
  // The return value is currently not used.

  return kTRUE;
}

void AnalysisSelector::Begin (TTree * /*tree*/) {
  // The Begin() function is called at the start of the query.
  // When running with PROOF Begin() is only called on the client.
  // The tree argument is deprecated (on PROOF 0 is passed).

  TString option = GetOption();
  
}

void AnalysisSelector::SlaveBegin (TTree * /*tree*/) {
  // The SlaveBegin() function is called after the Begin() function.
  // When running with PROOF SlaveBegin() is called on each slave server.
  // The tree argument is deprecated (on PROOF 0 is passed).

  TString option = GetOption();

}

Bool_t AnalysisSelector::Process (Long64_t entry) {
  // The Process() function is called for each entry in the tree (or possibly
  // keyed object in the case of PROOF) to be processed. The entry argument
  // specifies which entry in the currently loaded tree is to be processed.
  // It can be passed to either AnalysisSelector::GetEntry() or TBranch::GetEntry()
  // to read either all or the required parts of the data. When processing
  // keyed objects with PROOF, the object is already loaded and is available
  // via the fObject pointer.
  //
  // This function should contain the "body" of the analysis. It can contain
  // simple or elaborate selection criteria, run algorithms on the data
  // of the event and typically fill histograms.
  //
  // The processing can be stopped by calling Abort().
  //
  // Use fStatus to set the return value of TTree::Process().
  //
  // The return value is currently not used.
  GetEntry(entry);

  std::cout << "Beginning analysis flow." << std::endl;
  fAnalysisFlow->ExecuteTasks("");
  std::cout << "Finished analysis flow." << std::endl;

  return kTRUE;
}

void AnalysisSelector::SlaveTerminate () {
  // The SlaveTerminate() function is called after all entries or objects
  // have been processed. When running with PROOF SlaveTerminate() is called
  // on each slave server.

  // merge any flagged data in data map to fOutput
  // delete data map
}

void AnalysisSelector::Terminate () {
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.

  // loop through fOutput to save objects
}

} /* HAL */ 