#include <HAL/Algorithms.h>

namespace HAL
{

/*
 * Generic classes
 * */
internal::ImportParticleAlgo::ImportParticleAlgo (TString name, TString title) : 
  HAL::Algorithm(name, title), fIsCart(false), fIsE(false), fIsM(false), 
  fIsCartMET(false), fIsPhiEtMET(false), fHasCharge(false), fHasID(false) {

  fCartX0 = TString::Format("%s:x0", GetName().Data());
  fCartX1 = TString::Format("%s:x1", GetName().Data());
  fCartX2 = TString::Format("%s:x2", GetName().Data());
  fCartX3 = TString::Format("%s:x3", GetName().Data());
  fPt = TString::Format("%s:pt", GetName().Data());
  fEt = TString::Format("%s:et", GetName().Data());
  fEta = TString::Format("%s:eta", GetName().Data());
  fPhi = TString::Format("%s:phi", GetName().Data());
  fM = TString::Format("%s:m", GetName().Data());
  fE = TString::Format("%s:e", GetName().Data());
  fCharge = TString::Format("%s:charge", GetName().Data());
  fID = TString::Format("%s:id", GetName().Data());
  fNEntriesName = TString::Format("%s:nentries", GetName().Data());
}

void internal::ImportParticleAlgo::Init (Option_t* /*option*/) {
  HAL::AnalysisTreeReader *tr = GetRawData();

  if (tr->CheckBranchMapNickname(fCartX0) && tr->CheckBranchMapNickname(fCartX1) && 
      tr->CheckBranchMapNickname(fCartX2) && tr->CheckBranchMapNickname(fCartX3))
    fIsCart = true;
  else if (tr->CheckBranchMapNickname(fE) && tr->CheckBranchMapNickname(fPt) &&
           tr->CheckBranchMapNickname(fEta) && tr->CheckBranchMapNickname(fPhi))
    fIsE = true;
  else if (tr->CheckBranchMapNickname(fM) && tr->CheckBranchMapNickname(fPt) &&
           tr->CheckBranchMapNickname(fEta) && tr->CheckBranchMapNickname(fPhi))
    fIsM = true;
  else if (!tr->CheckBranchMapNickname(fCartX0) && tr->CheckBranchMapNickname(fCartX1) &&
           tr->CheckBranchMapNickname(fCartX2) && !tr->CheckBranchMapNickname(fCartX3))
    fIsCartMET = true;
  else if ((tr->CheckBranchMapNickname(fEt) || tr->CheckBranchMapNickname(fPt)) &&
           tr->CheckBranchMapNickname(fPhi) && !tr->CheckBranchMapNickname(fEta) &&
           !tr->CheckBranchMapNickname(fM) && !tr->CheckBranchMapNickname(fE))
    fIsPhiEtMET = true;
  if (!fIsCart && !fIsE && !fIsM && !fIsCartMET && !fIsPhiEtMET)
    HAL::HALException(GetName().Prepend("Couldn't determine how to import data: ").Data());

  if (tr->CheckBranchMapNickname(fCharge))
    fHasCharge = true;
  if (tr->CheckBranchMapNickname(fID))
    fHasID = true;
  // Use only one since these are synonyms
  if (tr->CheckBranchMapNickname(fEt))
    fPt = fEt;
}

void internal::ImportParticleAlgo::Exec (unsigned n) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeReader *tr = GetRawData();
  HAL::GenericData *gen_data = new GenericData(GetName(), true);

  data->SetValue(GetName(), gen_data);

  for (unsigned i = 0; i < n; ++i) {
    HAL::ParticlePtr particle = new HAL::Particle(GetName());
    TLorentzVector *vec = MakeTLV(i);

    particle->SetP (vec);
    if (fHasCharge) particle->SetCharge(tr->GetDecimal(fCharge, i));
    if (fHasID) particle->SetID(tr->GetInteger(fID, i));
    gen_data->AddParticle(particle);
    particle->SetOriginIndex(gen_data->GetNParticles() - 1);
    particle->SetOwnerIndex(gen_data->GetNParticles() - 1);
  }

  gen_data->SetRefType("none");
  IncreaseCounter(gen_data->GetNParticles());
}

void internal::ImportParticleAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

internal::ImportValueAlgo::ImportValueAlgo (TString name, TString title) :
  HAL::Algorithm(name, title) {

  fUserDataLabel = TString::Format("%s:value", name.Data());
}

void  internal::ImportValueAlgo::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::GenericData *gen_data = new GenericData(GetName());

  data->SetValue(GetName(), gen_data);

  StoreValue(gen_data);
}

void  internal::ImportValueAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

internal::AugmentValueAlgo::AugmentValueAlgo (TString name, TString title, 
    TString input, TString attribute_name) : 
  HAL::Algorithm(name, title), fInput(input), fAttributeLabel(attribute_name) {
}

void  internal::AugmentValueAlgo::Exec (Option_t* /*option*/) {
  HAL::AnalysisTreeReader *tr = GetRawData();
  HAL::AnalysisData *data = GetUserData();
  HAL::GenericData *gen_data = new GenericData(GetName(), true);
  HAL::GenericData *input_data = NULL;

  data->SetValue(GetName(), gen_data);

  if (data->Exists(fInput)) 
    input_data = (GenericData*)data->GetTObject(fInput);
  else
    return;

  for (ParticlePtrsIt particle = input_data->GetParticleBegin(); 
       particle != input_data->GetParticleEnd(); ++ particle) {
    ParticlePtr p = new Particle(**particle);
    StoreValue(tr, p, p->GetOriginIndex()); // must be origin index as this is branch index
    gen_data->AddParticle(p);
    p->SetOwner(GetName());
    p->SetOwnerIndex(gen_data->GetNParticles() - 1);
  }
  IncreaseCounter(gen_data->GetNParticles());
}

void internal::AugmentValueAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

void internal::NthElementAlgo::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::GenericData *gen_data = new GenericData(GetName());
  HAL::GenericData *input_data = NULL;
  HAL::GenericData *original_data = NULL;
  long long n, norigin;

  data->SetValue(GetName(), gen_data);

  if (data->Exists(fInput)) 
    input_data = (GenericData*)data->GetTObject(fInput);
  else
    return;

  // Need to look for actual owner for sorting purposes
  if (input_data->IsOwner())
    original_data = input_data;
  else if (input_data->GetNParticles() > 0)
    original_data = (GenericData*)data->GetTObject(input_data->GetOwner());
  else return;

  n = input_data->GetNParticles();
  norigin = original_data->GetNParticles();
  if (n < fN)
    return;

  // Create sorted list for original data
  if (!original_data->HasParticles(SortTag())) {
    HAL::ParticlePtrs sorted_particles;

    for (unsigned i = 0; i < norigin; ++i) 
      sorted_particles.push_back(original_data->GetParticle(i));
    Sort(sorted_particles);
    original_data->SetParticles(SortTag(), sorted_particles);
  }

  // loop over the sorted list and find the fN-th ranked member in fInput
  ParticlePtr result = NULL;
  ParticlePtrs sorted_list = original_data->GetParticles(SortTag());
  long long count = 0;
  for (ParticlePtrsIt particle = sorted_list.begin(); particle != sorted_list.end(); ++ particle) {
    for (ParticlePtrsIt my_particle = input_data->GetParticleBegin(); 
         my_particle != input_data->GetParticleEnd(); ++ my_particle) {
      if (*particle == *my_particle) {
        if (++count == fN) {
          result = *particle;
          break;
        }
      }
    }
    if (result != NULL)
      break;
  }

  gen_data->AddParticle(result);
  IncreaseCounter(gen_data->GetNParticles());
}

void internal::NthElementAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

void internal::FilterParticleAlgo::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::GenericData *gen_data = new GenericData(GetName());
  HAL::GenericData *input_data = NULL;

  data->SetValue(GetName(), gen_data);

  if (data->Exists(fInput)) 
    input_data = (GenericData*)data->GetTObject(fInput);
  else
    return;

  for (ParticlePtrsIt particle = input_data->GetParticleBegin(); 
       particle != input_data->GetParticleEnd(); ++ particle) {
    if (FilterPredicate(*particle))
      gen_data->AddParticle(*particle);
  }

  IncreaseCounter(gen_data->GetNParticles());
}

void internal::FilterParticleAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

void internal::FilterRefParticleAlgo::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::GenericData *gen_data = new GenericData(GetName());
  HAL::GenericData *input_data = NULL;
  HAL::GenericData *others_data = NULL;
  HAL::ParticlePtr  reference;

  data->SetValue(GetName(), gen_data);

  if (data->Exists(fInput) && data->Exists(fOthers)) {
    input_data = (GenericData*)data->GetTObject(fInput);
    others_data = (GenericData*)data->GetTObject(fOthers);
  }
  else
    return;

  for (ParticlePtrsIt particle = others_data->GetParticleBegin(); 
       particle != others_data->GetParticleEnd(); ++ particle) {
    bool add_particle = true;
    for (ParticlePtrsIt ref_particle = input_data->GetParticleBegin(); 
         ref_particle != input_data->GetParticleEnd(); ++ ref_particle) {
      reference = *ref_particle;
      if (reference == *particle || reference->HasSameParticles("parents", *particle))
        continue;
      if (!FilterPredicate(reference, *particle)) {
        add_particle = false;
        break;
      }
    }
    if (add_particle)
      gen_data->AddParticle(*particle);
  }

  IncreaseCounter(gen_data->GetNParticles());
}

void internal::FilterRefParticleAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

internal::FilterParentAlgo::FilterParentAlgo (TString name, TString title, 
    TString input, TString logic, long long length, ...) :
  Algorithm(name, title), fExclude(logic.EqualTo("ex", TString::kIgnoreCase)), fInput(input) {

  fRefNames = new const char*[length];
  va_list arguments;  // store the variable list of arguments

  va_start (arguments, length); // initializing arguments to store all values after length
  for (long long i = 0; i < length; ++i)
    fRefNames[i] = va_arg(arguments, const char*);
  va_end(arguments); // cleans up the list
}

void internal::FilterParentAlgo::Exec (Option_t* /*option*/) {
}

void internal::FilterParentAlgo::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

internal::ParticlesTLVStore::ParticlesTLVStore (TString name, TString title, TString input, TString bname) :
  Algorithm(name, title), fSearchedForAttributes(false), fBranchName(bname), fInput(input) {

  fNParticles = TString::Format("%s_n", fBranchName.Data());
}

void internal::ParticlesTLVStore::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeWriter *output = GetUserOutput();
  HAL::GenericData *input_data = NULL;
  long long i = 0;

  if (data->Exists(fInput))
    input_data = (GenericData*)data->GetTObject(fInput);
  else
    return;

  if (!fSearchedForAttributes) {
    i = 0;
    fSearchedForAttributes = true;
    for (ParticlePtrsIt particle = input_data->GetParticleBegin(); 
         particle != input_data->GetParticleEnd(); ++ particle) {
      for (std::map<TString, long double>::iterator attribute = (*particle)->GetAttributes().begin();
           attribute != (*particle)->GetAttributes().end(); ++attribute) {
        bool found = false;
        for (std::map<TString, bool>::iterator stored_attribute = fAttributeFlags.begin();
             stored_attribute != fAttributeFlags.end(); ++stored_attribute) {
          if (attribute->first.EqualTo(stored_attribute->first)) found = true;
        }
        if (i == 0 || (found && fAttributeFlags[attribute->first])) fAttributeFlags[attribute->first] = true;
        else fAttributeFlags[attribute->first] = false;
      }
      ++i;
    }
    for (std::map<TString, bool>::iterator stored_attribute = fAttributeFlags.begin();
         stored_attribute != fAttributeFlags.end(); ++stored_attribute) {
      fAttributeLabels[stored_attribute->first] = 
        TString::Format("%s_%s", fBranchName.Data(), stored_attribute->first.Data());
    }
  }

  i = 0;
  output->SetValue(fNParticles, input_data->GetNParticles());
  for (ParticlePtrsIt particle = input_data->GetParticleBegin(); 
      particle != input_data->GetParticleEnd(); ++ particle) 
    StoreValue(output, i++, (*particle));
}








/*
 * Actual classes
 * */

/*
 * Importing Algorithms
 * */

Algorithms::ImportParticle::ImportParticle (TString name, TString title, unsigned n) : 
  ImportParticleAlgo(name, title), fN(n) {
}

void Algorithms::ImportParticle::Exec (Option_t* /*option*/) {
  HAL::AnalysisTreeReader *tr = GetRawData();
  long long n = fN;

  // determine number of elements to read in
  if (n == 0) {
    if (tr->CheckBranchMapNickname(fNEntriesName)) {
      n = tr->GetInteger(fNEntriesName);
    }
    else if (fIsCart || fIsCartMET) {
      if (tr->GetRank(fCartX1) == 1)
        n = tr->GetDim(fCartX1);
      else if (tr->GetRank(fCartX1) == 0)
        n = 1;
    }
    else if (fIsE || fIsM || fIsPhiEtMET) {
      if (tr->GetRank(fPhi) == 1)
        n = tr->GetDim(fPhi);
      else if (tr->GetRank(fPhi) == 0)
        n = 1;
    }
  }
  else {
    if (tr->CheckBranchMapNickname(fNEntriesName) && 
        tr->GetInteger(fNEntriesName) < n) {
      n = tr->GetInteger(fNEntriesName);
    }
    else if (fIsCart || fIsCartMET) {
      if (tr->GetRank(fCartX1) == 1 && 
          tr->GetDim(fCartX1) < n)
        n = tr->GetDim(fCartX1);
      else if (tr->GetRank(fCartX1) == 0)
        n = 1;
    }
    else if (fIsE || fIsM || fIsPhiEtMET) {
      if (tr->GetRank(fPhi) == 1 && 
          tr->GetDim(fPhi) < n)
        n = tr->GetDim(fPhi);
      else if (tr->GetRank(fPhi) == 0)
        n = 1;
    }
  }
  // call actual Exec algo
  ImportParticleAlgo::Exec(n);
}

TLorentzVector* Algorithms::ImportParticle::MakeTLV (unsigned i) {
  HAL::AnalysisTreeReader *tr = GetRawData();

  if (fIsCart) {
    long double x0 = tr->GetDecimal(fCartX0, i),
                x1 = tr->GetDecimal(fCartX1, i),
                x2 = tr->GetDecimal(fCartX2, i),
                x3 = tr->GetDecimal(fCartX3, i);
    return new TLorentzVector(x1, x2, x3, x0);
  }
  else if (fIsE) {
    long double e = tr->GetDecimal(fE, i),
                pT = tr->GetDecimal(fPt, i),
                eta = tr->GetDecimal(fEta, i),
                phi = tr->GetDecimal(fPhi, i);
    return HAL::makeTLVFromPtEtaPhiE(pT, eta, phi, e);
  }
  else if (fIsM) {
    long double m = tr->GetDecimal(fM, i),
                pT = tr->GetDecimal(fPt, i),
                eta = tr->GetDecimal(fEta, i),
                phi = tr->GetDecimal(fPhi, i);
    return HAL::makeTLVFromPtEtaPhiM(pT, eta, phi, m);
  }
  else if (fIsCartMET) {
    long double x1 = tr->GetDecimal(fCartX1, i),
                x2 = tr->GetDecimal(fCartX2, i);
    return new TLorentzVector(x1, x2, 0.0, TMath::Sqrt(x1*x1 + x2*x2));
  }
  else if (fIsPhiEtMET) {
    long double phi = tr->GetDecimal(fPhi, i),
                pt = tr->GetDecimal(fPt, i);
    return new TLorentzVector(pt*TMath::Cos(phi), pt*TMath::Sin(phi), 0.0, pt);
  }
  throw HAL::HALException("Couldn't identify type in ImportTLV");
}

Algorithms::ImportBool::ImportBool (TString name, TString title) : 
  ImportValueAlgo(name, title) {

  fValue = TString::Format("%s:bool", name.Data());
}

void Algorithms::ImportBool::StoreValue (HAL::GenericData *gen_data) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeReader *tr = GetRawData();

  data->SetValue(fUserDataLabel, tr->GetBool(fValue));
  gen_data->SetRefName(fUserDataLabel);
  gen_data->SetRefType("bool");
}

Algorithms::ImportInteger::ImportInteger (TString name, TString title) : 
  ImportValueAlgo(name, title) {

  fValue = TString::Format("%s:integer", name.Data());
}

void Algorithms::ImportInteger::StoreValue (HAL::GenericData *gen_data) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeReader *tr = GetRawData();

  data->SetValue(fUserDataLabel, tr->GetInteger(fValue));
  gen_data->SetRefName(fUserDataLabel);
  gen_data->SetRefType("integer");
}

Algorithms::ImportCounting::ImportCounting (TString name, TString title) : 
  ImportValueAlgo(name, title) {

  fValue = TString::Format("%s:counting", name.Data());
}

void Algorithms::ImportCounting::StoreValue (HAL::GenericData *gen_data) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeReader *tr = GetRawData();

  data->SetValue(fUserDataLabel, tr->GetCounting(fValue));
  gen_data->SetRefName(fUserDataLabel);
  gen_data->SetRefType("counting");
}

Algorithms::ImportDecimal::ImportDecimal (TString name, TString title) : 
  ImportValueAlgo(name, title) {

  fValue = TString::Format("%s:decimal", name.Data());
}

void Algorithms::ImportDecimal::StoreValue (HAL::GenericData *gen_data) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeReader *tr = GetRawData();

  data->SetValue(fUserDataLabel, tr->GetDecimal(fValue));
  gen_data->SetRefName(fUserDataLabel);
  gen_data->SetRefType("decimal");
}

void Algorithms::AttachAttribute::StoreValue (AnalysisTreeReader *tr, 
                                               ParticlePtr particle, long long i) {
  if (fUserValue)
    particle->SetAttribute(fAttributeLabel, fValue);
  if (fBranchValue)
    particle->SetAttribute(fAttributeLabel, tr->GetDecimal(fBranchLabel, i));
  if (fPropertyValue) {
    if (fPtRank || fMRank || fERank || fEtRank || fP3Rank) {
      HAL::AnalysisData *data = GetUserData();
      HAL::GenericData *owner_data = NULL;
      HAL::GenericData *input_data = NULL;
      TString   sort_tag;
      long long norigin;

      if (fPtRank) sort_tag = "4v_pt";
      if (fMRank) sort_tag = "4v_m";
      if (fERank) sort_tag = "4v_e";
      if (fEtRank) sort_tag = "4v_et";
      if (fP3Rank) sort_tag = "4v_p3";

      // Need to look for actual owner for sorting purposes
      owner_data = (GenericData*)data->GetTObject(particle->GetOwner());

      norigin = owner_data->GetNParticles();

      // Create sorted list for owner data
      if (!owner_data->HasParticles(sort_tag)) {
        HAL::ParticlePtrs sorted_particles;

        for (unsigned j = 0; j < norigin; ++j) 
          sorted_particles.push_back(owner_data->GetParticle(j));
        Sort(sorted_particles);
        owner_data->SetParticles(sort_tag, sorted_particles);
      }

      // loop over the sorted list
      bool found = false;
      ParticlePtrs sorted_list = owner_data->GetParticles(sort_tag);
      long long origin_count = 0, input_count = 0;
      long long owner_index = particle->GetOwnerIndex();
      if (fRefCompare) input_data = (GenericData*)data->GetTObject(fRefParticles);
      else input_data = (GenericData*)data->GetTObject(fInput);
      for (ParticlePtrsIt sorted_particle = sorted_list.begin(); 
           sorted_particle != sorted_list.end(); ++ sorted_particle) {
        ++origin_count;
        for (ParticlePtrsIt my_particle = input_data->GetParticleBegin(); 
             my_particle != input_data->GetParticleEnd(); ++ my_particle) {
          if (*sorted_particle == *my_particle) {
            ++input_count;
            if (*sorted_particle == owner_data->GetParticle(owner_index)) {
              particle->SetAttribute(fAttributeLabel, input_count);
              found = true;
            }
            break;
          }
        }
        if (found) break;
      }
    }
  }
}

bool Algorithms::AttachAttribute::operator() (ParticlePtr lhs, ParticlePtr rhs) {
  TLorentzVector *lhs_vec = lhs->GetP();
  TLorentzVector *rhs_vec = rhs->GetP();

  if (fPtRank)
    return (lhs_vec->Pt() > rhs_vec->Pt());
  if (fMRank)
    return (lhs_vec->M() > rhs_vec->M());
  if (fERank)
    return (lhs_vec->E() > rhs_vec->E());
  if (fEtRank)
    return (lhs_vec->Et() > rhs_vec->Et());
  if (fP3Rank)
    return (lhs_vec->P() > rhs_vec->P());
  throw HALException(GetName().Prepend("Couldn't determine sorting information: "));
}

void Algorithms::AttachAttribute::Sort (ParticlePtrs &sl) {
  std::stable_sort(sl.begin(), sl.end(), *this);
}

/*
 * Reconstruction Algorithms
 * */

Algorithms::VecAddReco::VecAddReco (TString name, TString title, long long length, ...) :
    Algorithm(name, title), fLength(length) {
  fParentNames = new const char*[fLength];
  va_list arguments;  // store the variable list of arguments

  va_start (arguments, length); // initializing arguments to store all values after length
  for (long long i = 0; i < fLength; ++i)
    fParentNames[i] = va_arg(arguments, const char*);
  va_end(arguments); // cleans up the list
}

Algorithms::VecAddReco::~VecAddReco() {
  delete[] fParentNames;
}

void Algorithms::VecAddReco::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::GenericData *gen_data = new GenericData(GetName(), true);
  HAL::GenericData *input_data = NULL;
  std::set<std::set<ParticlePtr> > UniqueTuples;

  data->SetValue(GetName(), gen_data);

  // Find unique sets of tuples 
  // (relies on unique particles having unique addresses)
  for (long long i = 0; i < fLength; ++i) {
    if (data->Exists(fParentNames[i]))
      input_data = (GenericData*)data->GetTObject(fParentNames[i]);
    else
      return;

    std::set<std::set<ParticlePtr> > NewTuples;
    if (UniqueTuples.size() == 0) {
      // Loop over current set of particles and add either the particle 
      // or its parents to the ntuple
      for (ParticlePtrsIt particle = input_data->GetParticleBegin(); 
           particle != input_data->GetParticleEnd(); ++ particle) {

        bool                  unique = true;
        std::set<ParticlePtr> new_tuple;

        if ((*particle)->GetNParticles("parents") > 0) {
          ParticlePtrs parents = (*particle)->GetParticles("parents");
          for (ParticlePtrsIt parent = parents.begin(); 
              parent != parents.end(); ++ parent) {
            if (new_tuple.insert(*parent).second == false) {
              unique = false;
              break;
            }
          }
        }
        else {
          if (new_tuple.insert(*particle).second == false)
            continue;
        }

        if (unique)
          NewTuples.insert(new_tuple);
      }
    }
    else {
      for (std::set<std::set<ParticlePtr> >::iterator current_tuple = UniqueTuples.begin();
           current_tuple != UniqueTuples.end(); ++current_tuple) {

        // Loop over current set of particles and add either the particle 
        // or its parents to the ntuple
        for (ParticlePtrsIt particle = input_data->GetParticleBegin(); 
             particle != input_data->GetParticleEnd(); ++ particle) {

          bool                  unique = true;
          std::set<ParticlePtr> new_tuple = *current_tuple;

          if ((*particle)->GetNParticles("parents") > 0) {
            ParticlePtrs parents = (*particle)->GetParticles("parents");
            for (ParticlePtrsIt parent = parents.begin(); 
                 parent != parents.end(); ++ parent) {
              if (new_tuple.insert(*parent).second == false) {
                unique = false;
                break;
              }
            }
          }
          else {
            if (new_tuple.insert(*particle).second == false)
              continue;
          }

          if (unique)
            NewTuples.insert(new_tuple);
        }
      }
    }

    UniqueTuples = NewTuples;
  }

  // Loop over UniqueTuples to make vectors
  for (std::set<std::set<ParticlePtr> >::iterator current_tuple = UniqueTuples.begin();
       current_tuple != UniqueTuples.end(); ++current_tuple) {
    float new_charge = 0.0;
    HAL::ParticlePtr new_particle = new HAL::Particle(GetName());
    HAL::ParticlePtrs new_parents;
    TLorentzVector *vec = new TLorentzVector();
    for (std::set<ParticlePtr>::iterator particle = current_tuple->begin();
         particle != current_tuple->end(); ++particle) {
      new_charge += (*particle)->GetCharge();
      vec->operator+=(*((*particle)->GetP()));
      new_parents.push_back(*particle);
    }
    new_particle->SetCharge(new_charge);
    new_particle->SetP(vec);
    new_particle->SetParticles("parents", new_parents);
    gen_data->AddParticle(new_particle);
    new_particle->SetOwnerIndex(gen_data->GetNParticles() - 1);
    new_particle->SetOriginIndex(gen_data->GetNParticles() - 1);
  }
  gen_data->SetRefType("none");
}

void Algorithms::VecAddReco::Clear (Option_t* /*option*/) {
  delete GetUserData()->GetTObject(GetName());
}

Algorithms::ParticleRankSelection::ParticleRankSelection (TString name, TString title, 
    TString input, unsigned rank, TString property, TString end) : 
  NthElementAlgo(name, title, input, rank), fPt(false), fM(false), fE(false),
    fEt(false), fP3(false), fHigh(false), fLow(false),
    fTLVProperty(property), fEnd(end) {
  if (fTLVProperty.EqualTo("pt", TString::kIgnoreCase))
    fPt = true;
  else if (fTLVProperty.EqualTo("m", TString::kIgnoreCase))
    fM = true;
  else if (fTLVProperty.EqualTo("e", TString::kIgnoreCase))
    fE = true;
  else if (fTLVProperty.EqualTo("et", TString::kIgnoreCase))
    fEt = true;
  else if (fTLVProperty.EqualTo("p3", TString::kIgnoreCase))
    fP3 = true;

  if (fEnd.EqualTo("high", TString::kIgnoreCase))
    fHigh = true;
  else if (fEnd.EqualTo("low", TString::kIgnoreCase))
    fLow = true;
}

TString Algorithms::ParticleRankSelection::SortTag () {
  if (fPt)
    return "4v_pt";
  if (fM)
    return "4v_m";
  if (fE)
    return "4v_e";
  if (fEt)
    return "4v_et";
  if (fP3)
    return "4v_p3";
  throw HALException(GetName().Prepend("Couldn't determine sorting type: "));
}

bool Algorithms::ParticleRankSelection::operator() (ParticlePtr lhs, ParticlePtr rhs) {
  TLorentzVector *lhs_vec = lhs->GetP();
  TLorentzVector *rhs_vec = rhs->GetP();

  if (fHigh) {
    if (fPt)
      return (lhs_vec->Pt() > rhs_vec->Pt());
    if (fM)
      return (lhs_vec->M() > rhs_vec->M());
    if (fE)
      return (lhs_vec->E() > rhs_vec->E());
    if (fEt)
      return (lhs_vec->Et() > rhs_vec->Et());
    if (fP3)
      return (lhs_vec->P() > rhs_vec->P());
  }
  else if (fLow) {
    if (fPt)
      return (lhs_vec->Pt() < rhs_vec->Pt());
    if (fM)
      return (lhs_vec->M() < rhs_vec->M());
    if (fE)
      return (lhs_vec->E() < rhs_vec->E());
    if (fEt)
      return (lhs_vec->Et() < rhs_vec->Et());
    if (fP3)
      return (lhs_vec->P() < rhs_vec->P());
  }
  throw HALException(GetName().Prepend("Couldn't determine sorting information: "));
}

void Algorithms::ParticleRankSelection::Sort (ParticlePtrs &sl) {
  std::stable_sort(sl.begin(), sl.end(), *this);
}

Algorithms::SelectParticle::SelectParticle (TString name, TString title, TString input, 
    TString property, TString op, double value) : 
  FilterParticleAlgo(name, title, input), 
  fPt(false), fM(false), fE(false), fEt(false), fP3(false), fEta(false), 
  fPhi(false), fCharge(false), fID(false), fAttribute(false), fEqual(false), fNotEqual(false), 
  fLessThan(false), fGreaterThan(false), fLessThanEqual(false), 
  fGreaterThanEqual(false), fIn(false), fOut(false), fSingleEnd(true), fWindow(false), 
  fList(false), fProperty(property) {

  Setup();

  if (op.EqualTo("==") || op.EqualTo("=")) {
    fEqual = true;
    fLowLimit = value;
  }
  else if (op.EqualTo("!=")) {
    fNotEqual = true;
    fLowLimit = value;
  }
  else if (op.EqualTo(">")) {
    fGreaterThan = true;
    fLowLimit = value;
  }
  else if (op.EqualTo("<")) {
    fLessThan = true;
    fHighLimit = value;
  }
  else if (op.EqualTo(">=")) {
    fGreaterThanEqual = true;
    fLowLimit = value;
  }
  else if (op.EqualTo("<=")) {
    fLessThanEqual = true;
    fHighLimit = value;
  }
}

Algorithms::SelectParticle::SelectParticle (TString name, TString title, TString input, 
    TString property, TString inclusion, double low, double high) : 
  FilterParticleAlgo(name, title, input), 
  fHighLimit(high), fLowLimit(low), 
  fPt(false), fM(false), fE(false), fEt(false), fP3(false), fEta(false), 
  fPhi(false), fCharge(false), fID(false), fAttribute(false), fEqual(false), fNotEqual(false), 
  fLessThan(false), fGreaterThan(false), fLessThanEqual(false), 
  fGreaterThanEqual(false), fIn(false), fOut(false), fSingleEnd(false), fWindow(true), 
  fList(false), fProperty(property) {

  Setup();

  if (inclusion.EqualTo("inclusive", TString::kIgnoreCase) || 
      inclusion.EqualTo("in", TString::kIgnoreCase))
    fIn = true;
  else if (inclusion.EqualTo("exclusive", TString::kIgnoreCase) || 
           inclusion.EqualTo("out", TString::kIgnoreCase))
    fOut = true;
}

Algorithms::SelectParticle::SelectParticle (TString name, TString title, TString input, 
    TString property, int length, ...) : 
  FilterParticleAlgo(name, title, input), 
  fPt(false), fM(false), fE(false), fEt(false), fP3(false), fEta(false), 
  fPhi(false), fCharge(false), fID(false), fAttribute(false), fEqual(false), fNotEqual(false), 
  fLessThan(false), fGreaterThan(false), fLessThanEqual(false), 
  fGreaterThanEqual(false), fIn(false), fOut(false), fSingleEnd(false), fWindow(false), 
  fList(true), fProperty(property) {

  Setup();

  va_list arguments;  // store the variable list of arguments
  va_start (arguments, length); // initializing arguments to store all values after length
  for (long long i = 0; i < length; ++i) {
    if (fID)
      fListValues.push_back(va_arg(arguments, int));
    else
      fListValues.push_back(va_arg(arguments, double));
  }
  va_end(arguments); // cleans up the list
}

void Algorithms::SelectParticle::Setup () {
  if (fProperty.EqualTo("pt", TString::kIgnoreCase))
    fPt = true;
  else if (fProperty.EqualTo("m", TString::kIgnoreCase))
    fM = true;
  else if (fProperty.EqualTo("e", TString::kIgnoreCase))
    fE = true;
  else if (fProperty.EqualTo("et", TString::kIgnoreCase))
    fEt = true;
  else if (fProperty.EqualTo("p3", TString::kIgnoreCase))
    fP3 = true;
  else if (fProperty.EqualTo("eta", TString::kIgnoreCase))
    fEta = true;
  else if (fProperty.EqualTo("phi", TString::kIgnoreCase))
    fPhi = true;
  else if (fProperty.EqualTo("charge", TString::kIgnoreCase))
    fCharge = true;
  else if (fProperty.EqualTo("id", TString::kIgnoreCase))
    fID = true;
  else
    fAttribute = true;
}

bool Algorithms::SelectParticle::FilterPredicate(ParticlePtr particle) {
  TLorentzVector *vec = particle->GetP();
  double property = 0.0;

  if (fPt)
    property = vec->Pt();
  else if (fM)
    property = vec->M();
  else if (fE)
    property = vec->E();
  else if (fEt)
    property = vec->Et();
  else if (fP3)
    property = vec->P();
  else if (fEta)
    property = vec->Eta();
  else if (fPhi)
    property = vec->Phi();
  else if (fID)
    property = (double)particle->GetID();
  else if (fCharge)
    property = (double)particle->GetCharge();
  else if (fAttribute && particle->HasAttribute(fProperty))
    property = particle->GetAttribute(fProperty);
  else 
    throw HAL::HALException(GetName().Prepend("Couldn't determine property to filter: "));

  if (fSingleEnd) {
    if (fEqual) 
      return (property == fLowLimit);
    else if (fNotEqual) 
      return (property != fLowLimit);
    else if (fGreaterThan)
      return (property > fLowLimit);
    else if (fLessThan)
      return (property < fHighLimit);
    else if (fGreaterThanEqual)
      return (property >= fLowLimit);
    else if (fLessThanEqual)
      return (property <= fHighLimit);
  }

  else if (fWindow) {
    if (fIn)
      return (property <= fHighLimit && property >= fLowLimit);
    else if (fOut)
      return (property >= fHighLimit && property <= fLowLimit);
  }

  else if (fList) {
    for (std::vector<double>::iterator val = fListValues.begin();
        val != fListValues.end(); ++val) {
      if (property == *val)
        return true;
    }
    return false;
  }

  throw HAL::HALException(GetName().Prepend("Couldn't determine how to filter: "));
}

Algorithms::SelectRefParticle::SelectRefParticle (TString name, TString title, TString input, TString others, 
    double value, TString type, TString inclusion) : 
  FilterRefParticleAlgo(name, title, input, others), fIn(false), fOut(false), 
  fWindow(false), fDeltaR(false), fDeltaPhi(false) {

  if (inclusion.EqualTo("in", TString::kIgnoreCase) || 
      inclusion.EqualTo("inclusive", TString::kIgnoreCase)) {
    fIn = true;
    fHighLimit = value;
  }
  else if (inclusion.EqualTo("out", TString::kIgnoreCase) || 
           inclusion.EqualTo("exclusive", TString::kIgnoreCase)) {
    fOut = true;
    fLowLimit = value;
  }

  if (type.EqualTo("dr", TString::kIgnoreCase)) 
    fDeltaR = true;
  else if (type.EqualTo("dphi", TString::kIgnoreCase)) 
    fDeltaPhi = true;
}

Algorithms::SelectRefParticle::SelectRefParticle (TString name, TString title, TString input, TString others, 
    double low, double high, TString type, TString inclusion) :
  FilterRefParticleAlgo(name, title, input, others), fHighLimit(high), fLowLimit(low), fIn(false), fOut(false), 
  fWindow(true), fDeltaR(false), fDeltaPhi(false) {

  if (inclusion.EqualTo("in", TString::kIgnoreCase) || 
      inclusion.EqualTo("inclusive", TString::kIgnoreCase))
    fIn = true;
  else if (inclusion.EqualTo("out", TString::kIgnoreCase) || 
           inclusion.EqualTo("exclusive", TString::kIgnoreCase))
    fOut = true;

  if (type.EqualTo("dr", TString::kIgnoreCase)) 
    fDeltaR = true;
  else if (type.EqualTo("dphi", TString::kIgnoreCase)) 
    fDeltaPhi = true;
}

bool Algorithms::SelectRefParticle::FilterPredicate (HAL::ParticlePtr p_ref, HAL::ParticlePtr particle) {
  TLorentzVector *ref = p_ref->GetP();
  TLorentzVector *vec = particle->GetP();
  double val = 0.0;

  if (fDeltaR)
    val = ref->DeltaR(*vec);
  else if (fDeltaPhi)
    val = ref->DeltaPhi(*vec);

  if (fWindow) {
    if (fIn)
      return (val >= fLowLimit && val <= fHighLimit);
    else if (fOut)
      return (val <= fLowLimit && val >= fHighLimit);
  }
  else if (fIn)
    return (val <= fHighLimit);
  else if (fOut)
    return (val >= fLowLimit);
  throw HAL::HALException(GetName().Prepend("Couldn't determine how to filter: "));
}

Algorithms::MinChiSquaredSelection::MinChiSquaredSelection (TString name, TString title, 
    long long length, ...) :
  Algorithm(name, title) {
  va_list arguments;  // store the variable list of arguments

  va_start (arguments, length); // initializing arguments to store all values after length
  for (long long i = 0; i < length; ++i) {
    // input_algo, property, ref_algo, mean_value, sigma_value, group_num, output_name
    TString InputAlgo(va_arg(arguments, const char*));
    TString Property(va_arg(arguments, const char*));
    TString RefAlgo(va_arg(arguments, const char*));
    double MeanValue = va_arg(arguments, double);
    double SigmaValue = va_arg(arguments, double);
    int GroupNumber = va_arg(arguments, int);
    TString OutputName(va_arg(arguments, const char*));

  }
  va_end(arguments); // cleans up the list
}

/*
 * Cutting Algorithms
 * */

Algorithms::Cut::Cut (TString name, TString title, TString logic, 
    long long length, ...) :
  CutAlgorithm(name, title), fAnd(false), fOr(false) {
  va_list arguments;  // store the variable list of arguments

  if (logic.EqualTo("and", TString::kIgnoreCase))
    fAnd = true;
  else if (logic.EqualTo("or", TString::kIgnoreCase))
    fOr = true;
  va_start (arguments, length); // initializing arguments to store all values after length
  for (long long i = 0; i < length; ++i) {
    const char* algo_name = va_arg(arguments, const char*);
    TString Type(va_arg(arguments, const char*));
    TString Op(va_arg(arguments, const char*));

    if (Type.EqualTo("bool", TString::kIgnoreCase)) {
      BoolAlgoInfo *Algo = new BoolAlgoInfo();
      Algo->fValue = (bool)va_arg(arguments, int);
      Algo->fName = algo_name;
      if (Op.EqualTo("==") || Op.EqualTo("="))
        Algo->fEqual = true;
      else if (Op.EqualTo("!="))
        Algo->fNotEqual = true;
      else if (Op.EqualTo(">"))
        Algo->fGreaterThan = true;
      else if (Op.EqualTo("<"))
        Algo->fLessThan = true;
      else if (Op.EqualTo(">="))
        Algo->fGreaterThanEqual = true;
      else if (Op.EqualTo("<="))
        Algo->fLessThanEqual = true;
      fAlgorithms.push_back(Algo);
    }
    else if (Type.EqualTo("integer", TString::kIgnoreCase)) {
      IntegerAlgoInfo *Algo = new IntegerAlgoInfo();
      Algo->fValue = (long long)va_arg(arguments, int);
      Algo->fName = algo_name;
      if (Op.EqualTo("==") || Op.EqualTo("="))
        Algo->fEqual = true;
      else if (Op.EqualTo("!="))
        Algo->fNotEqual = true;
      else if (Op.EqualTo(">"))
        Algo->fGreaterThan = true;
      else if (Op.EqualTo("<"))
        Algo->fLessThan = true;
      else if (Op.EqualTo(">="))
        Algo->fGreaterThanEqual = true;
      else if (Op.EqualTo("<="))
        Algo->fLessThanEqual = true;
      fAlgorithms.push_back(Algo);
    }
    else if (Type.EqualTo("counting", TString::kIgnoreCase)) {
      CountingAlgoInfo *Algo = new CountingAlgoInfo();
      Algo->fValue = (unsigned long long)va_arg(arguments, unsigned int);
      Algo->fName = algo_name;
      if (Op.EqualTo("==") || Op.EqualTo("="))
        Algo->fEqual = true;
      else if (Op.EqualTo("!="))
        Algo->fNotEqual = true;
      else if (Op.EqualTo(">"))
        Algo->fGreaterThan = true;
      else if (Op.EqualTo("<"))
        Algo->fLessThan = true;
      else if (Op.EqualTo(">="))
        Algo->fGreaterThanEqual = true;
      else if (Op.EqualTo("<="))
        Algo->fLessThanEqual = true;
      fAlgorithms.push_back(Algo);
    }
    else if (Type.EqualTo("decimal", TString::kIgnoreCase)) {
      DecimalAlgoInfo *Algo = new DecimalAlgoInfo();
      Algo->fValue = (long double)va_arg(arguments, double);
      Algo->fName = algo_name;
      if (Op.EqualTo("==") || Op.EqualTo("="))
        Algo->fEqual = true;
      else if (Op.EqualTo("!="))
        Algo->fNotEqual = true;
      else if (Op.EqualTo(">"))
        Algo->fGreaterThan = true;
      else if (Op.EqualTo("<"))
        Algo->fLessThan = true;
      else if (Op.EqualTo(">="))
        Algo->fGreaterThanEqual = true;
      else if (Op.EqualTo("<="))
        Algo->fLessThanEqual = true;
      fAlgorithms.push_back(Algo);
    }
    else if (Type.EqualTo("particle", TString::kIgnoreCase)) {
      NParticlesAlgoInfo *Algo = new NParticlesAlgoInfo();
      Algo->fValue = (long long)va_arg(arguments, int);
      Algo->fName = algo_name;
      if (Op.EqualTo("==") || Op.EqualTo("="))
        Algo->fEqual = true;
      else if (Op.EqualTo("!="))
        Algo->fNotEqual = true;
      else if (Op.EqualTo(">"))
        Algo->fGreaterThan = true;
      else if (Op.EqualTo("<"))
        Algo->fLessThan = true;
      else if (Op.EqualTo(">="))
        Algo->fGreaterThanEqual = true;
      else if (Op.EqualTo("<="))
        Algo->fLessThanEqual = true;
      fAlgorithms.push_back(Algo);
    }
  }
  va_end(arguments); // cleans up the list
}

Algorithms::Cut::~Cut () {
  for (std::vector<AlgoInfo*>::iterator it = fAlgorithms.begin();
      it != fAlgorithms.end(); ++it) {
    delete *it;
  }
}

void Algorithms::Cut::Exec (Option_t* /*option*/) {
  AnalysisData *data = GetUserData();
  HAL::GenericData *input_data = NULL;

  if (fAnd) {
    for (std::vector<AlgoInfo*>::iterator it = fAlgorithms.begin();
         it != fAlgorithms.end(); ++it) {
      if (!data->Exists((*it)->fName)) {
        Abort();
        return;
      }
      input_data = (GenericData*)data->GetTObject((*it)->fName);
      if (!(*it)->Eval(data, input_data)) {
        Abort();
        return;
      }
    }
    Passed();
  }
  else if (fOr) {
    for (std::vector<AlgoInfo*>::iterator it = fAlgorithms.begin();
         it != fAlgorithms.end(); ++it) {
      if (data->Exists((*it)->fName)) {
        input_data = (GenericData*)data->GetTObject((*it)->fName);
        if ((*it)->Eval(data, input_data)) {
          Passed();
          return;
        }
      }
    }
    Abort();
  }
}

bool  Algorithms::Cut::BoolAlgoInfo::Eval (HAL::AnalysisData *data, HAL::GenericData *gen_data) {
  bool current_value = data->GetBool(gen_data->GetRefName());

  if (fEqual && current_value == fValue)
    return true;
  if (fNotEqual && current_value != fValue)
    return true;
  return false;
}

bool  Algorithms::Cut::IntegerAlgoInfo::Eval (HAL::AnalysisData *data, HAL::GenericData *gen_data) {
  long long current_value = data->GetInteger(gen_data->GetRefName());

  if (fEqual && current_value == fValue)
    return true;
  if (fNotEqual && current_value != fValue)
    return true;
  if (fLessThan && current_value < fValue)
    return true;
  if (fGreaterThan && current_value > fValue)
    return true;
  if (fLessThanEqual && current_value <= fValue)
    return true;
  if (fGreaterThanEqual && current_value >= fValue)
    return true;
  return false;
}

bool  Algorithms::Cut::CountingAlgoInfo::Eval (HAL::AnalysisData *data, HAL::GenericData *gen_data) {
  unsigned long long current_value = data->GetCounting(gen_data->GetRefName());

  if (fEqual && current_value == fValue)
    return true;
  if (fNotEqual && current_value != fValue)
    return true;
  if (fLessThan && current_value < fValue)
    return true;
  if (fGreaterThan && current_value > fValue)
    return true;
  if (fLessThanEqual && current_value <= fValue)
    return true;
  if (fGreaterThanEqual && current_value >= fValue)
    return true;
  return false;
}

bool  Algorithms::Cut::DecimalAlgoInfo::Eval (HAL::AnalysisData *data, HAL::GenericData *gen_data) {
  long double current_value = data->GetDecimal(gen_data->GetRefName());

  if (fEqual && current_value == fValue)
    return true;
  if (fNotEqual && current_value != fValue)
    return true;
  if (fLessThan && current_value < fValue)
    return true;
  if (fGreaterThan && current_value > fValue)
    return true;
  if (fLessThanEqual && current_value <= fValue)
    return true;
  if (fGreaterThanEqual && current_value >= fValue)
    return true;
  return false;
}

bool  Algorithms::Cut::NParticlesAlgoInfo::Eval (HAL::AnalysisData * /*data*/, HAL::GenericData *gen_data) {
  long long current_value = gen_data->GetNParticles();

  if (fEqual && current_value == fValue)
    return true;
  if (fNotEqual && current_value != fValue)
    return true;
  if (fLessThan && current_value < fValue)
    return true;
  if (fGreaterThan && current_value > fValue)
    return true;
  if (fLessThanEqual && current_value <= fValue)
    return true;
  if (fGreaterThanEqual && current_value >= fValue)
    return true;
  return false;
}

/*
 * Monitoring Algorithms
 * */

void Algorithms::MonitorAlgorithm::Exec (Option_t* /*option*/) {
  HAL::AnalysisData *data = GetUserData();
  HAL::AnalysisTreeReader *tr = GetRawData();
  HAL::GenericData *input_data = NULL;

  if ((tr->GetEntryNumber() + 1) % fN == 0) {
    (*fOS) << "\nSummary for algorithm - " << fInput.Data() << std::endl;

    if (data->Exists(fInput))
      input_data = (GenericData*)data->GetTObject(fInput);
    else {
      (*fOS) << "Algorithm is empty!" << std::endl;
      return;
    }

    (*fOS) << *input_data << std::endl;
  }
}

void Algorithms::MonitorUserData::Exec (Option_t* /*option*/) {
  //HAL::AnalysisData *data = GetUserData();

  (*fOS) << "UserData has the following elements:" << std::endl;
}

/*
 * Exporting Algorithms
 * */

Algorithms::StoreParticle::StoreParticle (TString name, TString title, TString input, 
    TString property, TString bname, TString tree) :
  ParticlesTLVStore(name, title, input, bname), fAll(false), fAttributes(false), fPt(false), fM(false), 
  fE(false), fEt(false), fP3(false), fEta(false), fPhi(false), fID(false), fCharge(false), 
  fTreeName(tree) {

  if (property.EqualTo("pt", TString::kIgnoreCase))
    fPt = true;
  else if (property.EqualTo("m", TString::kIgnoreCase))
    fM = true;
  else if (property.EqualTo("e", TString::kIgnoreCase))
    fE = true;
  else if (property.EqualTo("et", TString::kIgnoreCase))
    fEt = true;
  else if (property.EqualTo("p3", TString::kIgnoreCase))
    fP3 = true;
  else if (property.EqualTo("eta", TString::kIgnoreCase))
    fEta = true;
  else if (property.EqualTo("phi", TString::kIgnoreCase))
    fPhi = true;
  else if (property.EqualTo("id", TString::kIgnoreCase))
    fID = true;
  else if (property.EqualTo("charge", TString::kIgnoreCase))
    fCharge = true;
  else if (property.EqualTo("all", TString::kIgnoreCase))
    fAll = true;
  else if (property.EqualTo("attributes", TString::kIgnoreCase))
    fAttributes = true;

  fPtLabel = TString::Format("%s_pt", fBranchName.Data());
  fEtLabel = TString::Format("%s_et", fBranchName.Data());
  fP3Label = TString::Format("%s_p", fBranchName.Data());
  fEtaLabel = TString::Format("%s_eta", fBranchName.Data());
  fPhiLabel = TString::Format("%s_phi", fBranchName.Data());
  fMLabel = TString::Format("%s_m", fBranchName.Data());
  fELabel = TString::Format("%s_e", fBranchName.Data());
  fIDLabel = TString::Format("%s_id", fBranchName.Data());
  fChargeLabel = TString::Format("%s_charge", fBranchName.Data());
}

void  Algorithms::StoreParticle::Init (Option_t* /*option*/) {
  HAL::AnalysisTreeWriter *output = GetUserOutput();

  output->SetTreeForBranch(fTreeName, fNParticles);

  if (fPt) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fM) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fE) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fEt) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fP3) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fEta) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fPhi) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fID) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fCharge) {
    output->SetTreeForBranch(fTreeName, fBranchName);
    return;
  }
  if (fAll) {
    output->SetTreeForBranch(fTreeName, fPtLabel);
    output->SetTreeForBranch(fTreeName, fEtaLabel);
    output->SetTreeForBranch(fTreeName, fPhiLabel);
    output->SetTreeForBranch(fTreeName, fMLabel);
    output->SetTreeForBranch(fTreeName, fELabel);
    output->SetTreeForBranch(fTreeName, fIDLabel);
    output->SetTreeForBranch(fTreeName, fChargeLabel);
    return;
  }
  if (fAttributes) { // Can't assign tree hear b/c attributes don't exist yet
    return;
  }
}

void Algorithms::StoreParticle::StoreValue (HAL::AnalysisTreeWriter *output, long long i, HAL::ParticlePtr particle) {
  TLorentzVector *vec = particle->GetP();

  if (fPt) {
    output->SetValue(fBranchName, vec->Pt(), i);
    return;
  }
  if (fM) {
    output->SetValue(fBranchName, vec->M(), i);
    return;
  }
  if (fE) {
    output->SetValue(fBranchName, vec->E(), i);
    return;
  }
  if (fEt) {
    output->SetValue(fBranchName, vec->Et(), i);
    return;
  }
  if (fP3) {
    output->SetValue(fBranchName, vec->P(), i);
    return;
  }
  if (fEta) {
    output->SetValue(fBranchName, vec->Eta(), i);
    return;
  }
  if (fPhi) {
    output->SetValue(fBranchName, vec->Phi(), i);
    return;
  }
  if (fID) {
    output->SetValue(fBranchName, particle->GetID(), i);
    return;
  }
  if (fCharge) {
    output->SetValue(fBranchName, particle->GetCharge(), i);
    return;
  }
  if (fAll) {
    for (std::map<TString, TString>::iterator it = fAttributeLabels.begin();
         it != fAttributeLabels.end(); ++it) {
      output->SetTreeForBranch(fTreeName, it->second);
      output->SetValue(it->second, particle->GetAttribute(it->first), i);
    }
    output->SetValue(fPtLabel, vec->Pt(), i);
    output->SetValue(fEtaLabel, vec->Eta(), i);
    output->SetValue(fPhiLabel, vec->Phi(), i);
    output->SetValue(fMLabel, vec->M(), i);
    output->SetValue(fELabel, vec->E(), i);
    output->SetValue(fIDLabel, particle->GetID(), i);
    output->SetValue(fChargeLabel, particle->GetCharge(), i);
    return;
  }
  if (fAttributes) {
    for (std::map<TString, TString>::iterator it = fAttributeLabels.begin();
         it != fAttributeLabels.end(); ++it) {
      output->SetTreeForBranch(fTreeName, it->second);
      output->SetValue(it->second, particle->GetAttribute(it->first), i);
    }
    return;
  }
  throw HAL::HALException(GetName().Prepend("Couldn't determine what to store: "));
}

} /* HAL */ 
