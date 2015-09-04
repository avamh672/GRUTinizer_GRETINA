#include "TRootOutfile.h"

#include "Globals.h"

#include "TDetectorEnv.h"
#include "TGRUTOptions.h"
#include "TJanus.h"
#include "TSega.h"
#include "TNSCLEvent.h"

ClassImp(TRootOutfile)

TRootOutfile::TRootOutfile() {
  outfile     = NULL;
}

TRootOutfile::~TRootOutfile() {
  for(auto list : hist_list){
    delete list.second;
  }

  if(outfile){
    FinalizeFile();
    CloseFile();
    //outfile->Close();
    //delete outfile;
  }
}

TTree *TRootOutfile::AddTree(const char* tname,const char* ttitle,bool build) {
  if(!outfile) {
    fprintf(stderr,"%s, attempting to make tree with out associated file.\n",__PRETTY_FUNCTION__);
  }
  if(!ttitle)
    ttitle = tname;
  outfile->cd();

  tree_element elem;
  elem.tree = new TTree(tname,ttitle);
  elem.build_det = build;
  trees[tname] = elem;
  return elem.tree;
};

TTree *TRootOutfile::FindTree(const char *tname) {
  try{
    return trees.at(tname).tree;
  } catch (std::out_of_range& e) {
    return NULL;
  }
}

void TRootOutfile::AddRawData(const TRawEvent& event, kDetectorSystems det_type){
  try{
    TDetector* det = det_list.at(det_type);
    det->AddRawData(event);
  } catch (std::out_of_range& e) { }
}

TRootOutfile::tree_element* TRootOutfile::FindTreeElement(const char* tname){
  try{
    return &trees.at(tname);
  } catch (std::out_of_range& e) {
    return NULL;
  }
}

void TRootOutfile::FillTree(const char *tname) {

  tree_element* elem = FindTreeElement(tname);
  if(!elem) {
    fprintf(stderr,"%s: trying to fill nonexisting tree %s\n.",__PRETTY_FUNCTION__,tname);
    return;
  }

  if(elem->build_det){
    for(auto& item : det_list) {
      item.second->Build();
    }
  }
  elem->tree->Fill();
}


void TRootOutfile::FillAllTrees() {
  for(auto& val : trees){
    tree_element& elem = val.second;
    if(elem.build_det){
      for(auto& item : det_list) {
        item.second->Build();
      }
    }

    elem.tree->Fill();
  }
}


void TRootOutfile::Clear(Option_t *opt) {
  for(auto& item : det_list) {
    item.second->Clear();
  }
}

void TRootOutfile::FinalizeFile(){
  if(outfile){
    outfile->cd();
  } else {
    return;
  }

  int counter =0;
  printf("Writing trees.\n"); fflush(stdout);
  for(auto& elem : trees){
    TTree* tree = elem.second.tree;
    printf("\t%s[%i] written...",tree->GetName(),tree->GetEntries());  fflush(stdout);
    tree->Write();
    printf(" done.\n"); fflush(stdout);
    counter++;
  }
  printf("done. %i trees written.\n",counter); fflush(stdout);

  counter=0;
  printf("Writing hists...\n"); fflush(stdout);
  for(auto& item:hist_list) {
    printf("\t%s:%s...",outfile->GetName(),item.first.c_str()); fflush(stdout);
    outfile->cd();
    if(item.first.length()>0) {
      outfile->mkdir(item.first.c_str());
      outfile->cd(item.first.c_str());
    }
    item.second->Sort();
    item.second->Write();
    printf(" %i objects written.i\n",item.second->GetSize()); fflush(stdout);
    counter++;
  }
  printf("done. %i dirs written.\n",counter); fflush(stdout);
  CloseFile();
}

void TRootOutfile::CloseFile(){
  outfile->Close();
  outfile->Delete();
  outfile = NULL;
}



void TRootOutfile::Print(Option_t* opt) const {

  return;
}