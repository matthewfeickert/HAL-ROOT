/*
 * Generic algorithms for fast analysis development
 *
 * DEV NOTE:
 * Importing and Reconstruction Algorithms create and destroy objects in 'data'
 *  - creates <name>:4-vec 1D array
 * All other Algorithms should never create or destroy objects in 'data'
 *  - creates <name>:ref_name scalar that points to the actual data (index refers to ref_name indices)
 * ref_names should always reference direct access data, never reference a reference
 * */

#ifndef HAL_ALGORITHMS
#define HAL_ALGORITHMS

#include <TRandom3.h>
#include <TString.h>
#include <TLorentzVector.h>
#include <cstdarg>
#include <string>
#include <deque>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <HAL/Common.h>
#include <HAL/Exceptions.h>
#include <HAL/AnalysisUtils.h>
#include <HAL/Algorithm.h>
#include <HAL/CutAlgorithm.h>
#include <HAL/AnalysisData.h>
#include <HAL/AnalysisTreeReader.h>
#include <HAL/AnalysisTreeWriter.h>

#include <iostream>

namespace HAL
{

/*
 * Generic base class algorithms and functions
 * Most of these are ABC's
 * */

namespace internal
{

/*
 * General function for determining how to access information
 * stored in AnalysisData (either reference or direct access)
 * */
bool determineAccessProtocol(HAL::AnalysisData *data, TString &RawInput, TString &RealInput);

/*
 * Algorithm for importing an array of TLorentzVecotr's from and TTree.
 * */
class ImportTLVAlgo : public HAL::Algorithm {
public:
  ImportTLVAlgo (TString name, TString title) : HAL::Algorithm(name, title) {}
  virtual ~ImportTLVAlgo () {}

protected:
  virtual void Exec (Option_t* /*option*/) {}
  virtual void Exec (unsigned n);
  virtual void Clear (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned) = 0;
};

/*
 * Algorithm for finding the nth highest/lowest element in an AnalysisData object
 * */
class NthElementAlgo : public HAL::Algorithm {
public:
  NthElementAlgo (TString name, TString title, TString input, unsigned n) :
    HAL::Algorithm(name, title), fN(n), fInput(input) {}
  virtual ~NthElementAlgo () {}

  virtual TString   SortTag () = 0;
  virtual bool      operator() (long long, long long) = 0;
  virtual void      Sort (std::vector<long long>&) = 0;

protected:
  virtual void      Exec (Option_t* /*option*/);
  virtual void      Clear (Option_t* /*option*/);

  unsigned  fN;
  TString   fInput, fElementName;
};

/*
 * Algorithm for filtering many particle by their TLV
 * */
class FilterTLVAlgo : public Algorithm {
public:
  FilterTLVAlgo (TString name, TString title, TString input) :
    Algorithm(name, title), fInput(input) {}
  virtual ~FilterTLVAlgo () {}

  virtual bool FilterPredicate (TLorentzVector*) = 0;
protected:
  virtual void Exec (Option_t* /*option*/);
  virtual void Clear (Option_t* /*option*/);

  TString fInput;
};

/*
 * Algorithm for cutting many particle on their TLV
 * */
class ParticlesTLVCut : public CutAlgorithm {
public:
  ParticlesTLVCut (TString name, TString title, TString input) :
    CutAlgorithm(name, title), fInput(input) {}
  virtual ~ParticlesTLVCut () {}

  // this should return true if particle passed cut
  virtual bool CutPredicate (TLorentzVector*) = 0;

protected:
  virtual void Exec (Option_t* /*option*/);

  TString   fInput;
};

/*
 * Algorithm for the exporting of simple quantities from a
 * particle's TLV
 * */
class SingleParticleTLVStore : public Algorithm {
public:
  SingleParticleTLVStore (TString name, TString title, TString input, TString bname) :
    Algorithm(name, title), fBranchName(bname), fInput(input) {}
  virtual ~SingleParticleTLVStore () {}


protected:
  virtual void    Exec (Option_t* /*option*/);
  virtual double  StoreValue (TLorentzVector*) = 0;

  TString         fBranchName, fInput;
};

/*
 * Algorithm for the exporting of simple quantities from
 * several particles' TLVs
 * */
class ParticlesTLVStore : public Algorithm {
public:
  ParticlesTLVStore (TString name, TString title, TString input, TString bname) :
    Algorithm(name, title), fBranchName(bname), fInput(input) {}
  virtual ~ParticlesTLVStore () {}


protected:
  virtual void    Exec (Option_t* /*option*/);
  virtual double  StoreValue (TLorentzVector*) = 0;

  TString         fBranchName, fInput;
};

} /* internal */ 



/*
 * Importing algorithms
 * */

/*
 * Import algorithm to build particles from the cartesian components
 * of a 4-vector.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:nentries
 *  <name>:x0
 *  <name>:x1
 *  <name>:x2
 *  <name>:x3
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0000 : public internal::ImportTLVAlgo {
public:
  IA0000 (TString name, TString title) : ImportTLVAlgo(name, title) {}
  virtual ~IA0000 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);
};


/*
 * Import algorithm to build particles from the cartesian components
 * of a 4-vector. It uses the dimension of the x1 coordinate array
 * as the number to loop over. This saves the user from having to
 * explicitly give an array length.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:x0
 *  <name>:x1
 *  <name>:x2
 *  <name>:x3
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0001 : public internal::ImportTLVAlgo {
public:
  IA0001 (TString name, TString title) : ImportTLVAlgo(name, title) {}
  virtual ~IA0001 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);
};


/*
 * Import algorithm to build particles from the cartesian components
 * of a 4-vector. User must specify number of particles to read in.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:x0
 *  <name>:x1
 *  <name>:x2
 *  <name>:x3
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0002 : public internal::ImportTLVAlgo {
public:
  IA0002 (TString name, TString title, unsigned n) : ImportTLVAlgo(name, title), fNEntries(n) {}
  virtual ~IA0002 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);

private:
  unsigned fNEntries;
};

/*
 * Import algorithm to build particles from the pT, eta, phi, and 
 * energy components of a 4-vector.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:nentries
 *  <name>:pt
 *  <name>:eta
 *  <name>:phi
 *  <name>:e
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0010 : public internal::ImportTLVAlgo {
public:
  IA0010 (TString name, TString title) : ImportTLVAlgo(name, title) {}
  virtual ~IA0010 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);
};


/*
 * Import algorithm to build particles from the pT, eta, phi, and 
 * energy components of a 4-vector. It uses the dimension of the pT 
 * array as the number to loop over. This saves the user from having 
 * to explicitly give an array length.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:pt
 *  <name>:eta
 *  <name>:phi
 *  <name>:e
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0011 : public internal::ImportTLVAlgo {
public:
  IA0011 (TString name, TString title) : ImportTLVAlgo(name, title) {}
  virtual ~IA0011 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);
};


/*
 * Import algorithm to build particles from the pT, eta, phi, and 
 * energy components of a 4-vector. User must specify number of 
 * particles to read in.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:pt
 *  <name>:eta
 *  <name>:phi
 *  <name>:e
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0012 : public internal::ImportTLVAlgo {
public:
  IA0012 (TString name, TString title, unsigned n) : ImportTLVAlgo(name, title), fNEntries(n) {}
  virtual ~IA0012 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);

private:
  unsigned fNEntries;
};


/*
 * Import algorithm to build particles from the pT, eta, phi, and 
 * mass components of a 4-vector.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:nentries
 *  <name>:pt
 *  <name>:eta
 *  <name>:phi
 *  <name>:m
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0020 : public internal::ImportTLVAlgo {
public:
  IA0020 (TString name, TString title) : ImportTLVAlgo(name, title) {}
  virtual ~IA0020 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);
};


/*
 * Import algorithm to build particles from the pT, eta, phi, and 
 * mass components of a 4-vector. It uses the dimension of the pT 
 * array as the number to loop over. This saves the user from having 
 * to explicitly give an array length.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:pt
 *  <name>:eta
 *  <name>:phi
 *  <name>:m
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0021 : public internal::ImportTLVAlgo {
public:
  IA0021 (TString name, TString title) : ImportTLVAlgo(name, title) {}
  virtual ~IA0021 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);
};


/*
 * Import algorithm to build particles from the pT, eta, phi, and 
 * mass components of a 4-vector. User must specify number of 
 * particles to read in.
 *
 * Prerequisites:
 *  None
 * Branch Maps Needed:
 *  <name>:pt
 *  <name>:eta
 *  <name>:phi
 *  <name>:m
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices (used in subsequent algorithms))
 */
class IA0022 : public internal::ImportTLVAlgo {
public:
  IA0022 (TString name, TString title, unsigned n) : ImportTLVAlgo(name, title), fNEntries(n) {}
  virtual ~IA0022 () {}

protected:
  using ImportTLVAlgo::Exec;
  virtual void Exec (Option_t* /*option*/);
  virtual TLorentzVector* MakeTLV (unsigned);

private:
  unsigned fNEntries;
};




/*
 * Reconstruction Algorithms
 * */

/*
 * Reconstruction algorithm to build particles from the
 * vector addition of other particles.
 * The unknown parameters should be a series of string literals
 *
 * Prerequisites:
 *  Stored Particles
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:nparents (scalar: number of parent particles)
 *  <name>:4-vec (1D array: of TLorentzVectors)
 *  <name>:index (1D array: of indices)
 *  <name>:parent_ref_name (1D array: ref_names of parents)
 *  <name>:parent_index (1D array: indices of parents)
 */
class RA0000: public Algorithm {
public:
  RA0000 (TString name, TString title, long long length, ...);
  virtual ~RA0000();

  virtual void  Exec (Option_t* /*option*/);
  virtual void  Clear (Option_t* /*option*/);
private:
  const char**  fParentNames;
  long long     fLength;
};



/*
 * Filtering Algorithms
 * */

/*
 * Select the particle with nth highest pT
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices (just one index that points to the nth highest pT of ref_name))
 * */
class FA0000 : public internal::NthElementAlgo {
public:
  FA0000 (TString name, TString title, TString input, unsigned n) : 
    NthElementAlgo(name, title, input, n) {}
  virtual ~FA0000 () {}

  virtual TString       SortTag ();
  virtual bool          operator() (long long, long long);
  virtual void          Sort (std::vector<long long>&);
};


/*
 * Select the particles with pT greater than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0100 : public internal::FilterTLVAlgo {
public:
  FA0100 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0100 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with eta greater than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0101 : public internal::FilterTLVAlgo {
public:
  FA0101 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0101 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with phi greater than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0102 : public internal::FilterTLVAlgo {
public:
  FA0102 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0102 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with mass greater than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0103 : public internal::FilterTLVAlgo {
public:
  FA0103 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0103 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with energy greater than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0104 : public internal::FilterTLVAlgo {
public:
  FA0104 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0104 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with pT less than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0110 : public internal::FilterTLVAlgo {
public:
  FA0110 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0110 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with eta less than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0111 : public internal::FilterTLVAlgo {
public:
  FA0111 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0111 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with phi less than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0112 : public internal::FilterTLVAlgo {
public:
  FA0112 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0112 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with mass less than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0113 : public internal::FilterTLVAlgo {
public:
  FA0113 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0113 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles with energy less than or equal to
 * a given value
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0114 : public internal::FilterTLVAlgo {
public:
  FA0114 (TString name, TString title, TString input, double v) : 
    FilterTLVAlgo(name, title, input), fValue(v) {}
  virtual ~FA0114 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fValue;
};


/*
 * Select the particles within a pT window
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0120 : public internal::FilterTLVAlgo {
public:
  FA0120 (TString name, TString title, TString input, double low, double high) : 
    FilterTLVAlgo(name, title, input), fLow(low), fHigh(high) {}
  virtual ~FA0120 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fLow, fHigh;
};


/*
 * Select the particles within a eta window
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0121 : public internal::FilterTLVAlgo {
public:
  FA0121 (TString name, TString title, TString input, double low, double high) : 
    FilterTLVAlgo(name, title, input), fLow(low), fHigh(high) {}
  virtual ~FA0121 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fLow, fHigh;
};


/*
 * Select the particles within a phi window
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0122 : public internal::FilterTLVAlgo {
public:
  FA0122 (TString name, TString title, TString input, double low, double high) : 
    FilterTLVAlgo(name, title, input), fLow(low), fHigh(high) {}
  virtual ~FA0122 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fLow, fHigh;
};


/*
 * Select the particles within a mass window
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0123 : public internal::FilterTLVAlgo {
public:
  FA0123 (TString name, TString title, TString input, double low, double high) : 
    FilterTLVAlgo(name, title, input), fLow(low), fHigh(high) {}
  virtual ~FA0123 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fLow, fHigh;
};


/*
 * Select the particles within a energy window
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  <name>:nobjects (scalar: number of particles)
 *  <name>:ref_name ((scalar): string name of reference particles to use)
 *  <name>:index (1D array: of indices)
 * */
class FA0124 : public internal::FilterTLVAlgo {
public:
  FA0124 (TString name, TString title, TString input, double low, double high) : 
    FilterTLVAlgo(name, title, input), fLow(low), fHigh(high) {}
  virtual ~FA0124 () {}

  virtual bool FilterPredicate(TLorentzVector*);

private:
  double fLow, fHigh;
};




/*
 * Cutting Algorithms
 * */

/*
 * Cut on particles' pT (lower limit)
 * (logical 'and')
 *
 * Prerequisites:
 *  Stored particle (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class CA0000 : public internal::ParticlesTLVCut {
public:
  CA0000 (TString name, TString title, TString input, double cut) :
    ParticlesTLVCut(name, title, input), fCutValue(cut) {}
  virtual ~CA0000 () {}

  virtual bool CutPredicate (TLorentzVector *vec);

private:
  double    fCutValue;
};


/*
 * Cut on particles' mass (lower limit)
 * (logical 'and')
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class CA0003 : public internal::ParticlesTLVCut {
public:
  CA0003 (TString name, TString title, TString input, double cut) :
    ParticlesTLVCut(name, title, input), fCutValue(cut) {}
  virtual ~CA0003 () {}

  virtual bool CutPredicate (TLorentzVector *vec);

private:
  double    fCutValue;
};


/*
 * Cut on number of particles
 * (logical 'and')
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class CA0100 : public CutAlgorithm {
public:
  CA0100 (TString name, TString title, long long n, long long length, ...);
  virtual ~CA0100 () {}

protected:
  virtual void Exec (Option_t* /*option*/);

private:
  long long     fLength, fN;
  const char**  fParticleNames;
};


/*
 * Cut on number of particles
 * (logical 'or')
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class CA0101 : public CutAlgorithm {
public:
  CA0101 (TString name, TString title, long long n, long long length, ...);
  virtual ~CA0101 () {}

protected:
  virtual void Exec (Option_t* /*option*/);

private:
  long long     fLength, fN;
  const char**  fParticleNames;
};




/*
 * Exporting Algorithms
 * */

/*
 * Store the pT of a particle
 *
 * Prerequisites:
 *  Stored particle (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0000 : public internal::SingleParticleTLVStore {
public:
  EA0000 (TString name, TString title, TString input, TString bname) :
    SingleParticleTLVStore(name, title, input, bname) {}
  virtual ~EA0000 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the eta of a particle
 *
 * Prerequisites:
 *  Stored particle (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0001 : public internal::SingleParticleTLVStore {
public:
  EA0001 (TString name, TString title, TString input, TString bname) :
    SingleParticleTLVStore(name, title, input, bname) {}
  virtual ~EA0001 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the phi of a particle
 *
 * Prerequisites:
 *  Stored particle (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0002 : public internal::SingleParticleTLVStore {
public:
  EA0002 (TString name, TString title, TString input, TString bname) :
    SingleParticleTLVStore(name, title, input, bname) {}
  virtual ~EA0002 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the mass of a particle
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0003 : public internal::SingleParticleTLVStore {
public:
  EA0003 (TString name, TString title, TString input, TString bname) :
    SingleParticleTLVStore(name, title, input, bname) {}
  virtual ~EA0003 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the energy of a particle
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0004 : public internal::SingleParticleTLVStore {
public:
  EA0004 (TString name, TString title, TString input, TString bname) :
    SingleParticleTLVStore(name, title, input, bname) {}
  virtual ~EA0004 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the pT of particles
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0010 : public internal::ParticlesTLVStore {
public:
  EA0010 (TString name, TString title, TString input, TString bname) :
    ParticlesTLVStore(name, title, input, bname) {}
  virtual ~EA0010 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the eta of particles
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0011 : public internal::ParticlesTLVStore {
public:
  EA0011 (TString name, TString title, TString input, TString bname) :
    ParticlesTLVStore(name, title, input, bname) {}
  virtual ~EA0011 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the phi of particles
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0012 : public internal::ParticlesTLVStore {
public:
  EA0012 (TString name, TString title, TString input, TString bname) :
    ParticlesTLVStore(name, title, input, bname) {}
  virtual ~EA0012 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the mass of particles
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0013 : public internal::ParticlesTLVStore {
public:
  EA0013 (TString name, TString title, TString input, TString bname) :
    ParticlesTLVStore(name, title, input, bname) {}
  virtual ~EA0013 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};


/*
 * Store the energy of particles
 *
 * Prerequisites:
 *  Stored particles (either as references or direct access)
 * Branch Maps Needed:
 *  None
 * Output:
 *  None
 * */
class EA0014 : public internal::ParticlesTLVStore {
public:
  EA0014 (TString name, TString title, TString input, TString bname) :
    ParticlesTLVStore(name, title, input, bname) {}
  virtual ~EA0014 () {}

protected:
  virtual double  StoreValue (TLorentzVector*);
};

} /* HAL */ 

#endif /* end of include guard: HAL_ALGORITHMS */

