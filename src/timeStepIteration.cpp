// Copyright (C) 2020, J. M. Perez Zerpa
//
// This file is part of ONSAS++.
//
// ONSAS++ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ONSAS is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ONSAS++.  If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <armadillo>

using namespace std  ;
using namespace arma ;

// =============================================================================
// nodes2dofs
// =============================================================================
ivec nodes2dofs( ivec nodes, int degreesPerNode ){

  int  n    = nodes.n_elem ;
  ivec dofs = zeros<ivec>( degreesPerNode * n ) ;
  
  for ( int i=0; i<n ; i++){
    for ( int j=0; j< degreesPerNode; j++){
      dofs( i * degreesPerNode + j ) = degreesPerNode*( nodes(i) - 1 ) + ( j+1 ) ;
    }
  }  
  return dofs;
}
// =============================================================================




// =============================================================================
// shapeFunsDeriv
// =============================================================================
mat shapeFunsDeriv ( double x, double y, double z ){
  mat fun = zeros<mat>( 4, 3 ) ;
  fun( 0, 0 ) =  1 ;
  fun( 1, 0 ) = -1 ;
  fun( 1, 1 ) = -1 ;
  fun( 1, 2 ) = -1 ;
  fun( 2, 2 ) =  1 ;
  fun( 3, 1 ) =  1 ;
  return fun;
}
// ======================================================================




// ======================================================================
// cosseratSVK
// ======================================================================
void cosseratSVK ( vec consParams, mat Egreen, int consMatFlag, mat & S, mat & ConsMat ){

  double young  = consParams(1-1) ;
  double nu     = consParams(2-1) ;
  
  double lambda = young * nu / ( (1 + nu) * (1 - 2*nu) ) ;
  double shear  = young      / ( 2 * (1 + nu) )          ;
  
  S      = lambda * trace(Egreen) * eye(3,3) + 2 * shear * Egreen ;
  ConsMat.zeros();

  if (consMatFlag == 1){ // complex-step computation expression
    //~ //ConsMat = zeros(6,6);
    //~ //ConsMat = complexStepConsMat( 'cosseratSVK', consParams, Egreen ) ;
  }else if (consMatFlag == 2){ // analytical expression
    ConsMat (1-1,1-1) = ( shear / (1 - 2 * nu) ) * 2 * ( 1-nu  ) ; 
    ConsMat (1-1,2-1) = ( shear / (1 - 2 * nu) ) * 2 * (   nu  ) ; 
    ConsMat (1-1,3-1) = ( shear / (1 - 2 * nu) ) * 2 * (   nu  ) ; 
  
    ConsMat (2-1,1-1) = ( shear / (1 - 2 * nu) ) * 2 * (    nu ) ;
    ConsMat (2-1,2-1) = ( shear / (1 - 2 * nu) ) * 2 * (  1-nu ) ;
    ConsMat (2-1,3-1) = ( shear / (1 - 2 * nu) ) * 2 * (    nu ) ;
    
    ConsMat (3-1,1-1) = ( shear / (1 - 2 * nu) ) * 2 * (   nu ) ;
    ConsMat (3-1,2-1) = ( shear / (1 - 2 * nu) ) * 2 * (   nu ) ;
    ConsMat (3-1,3-1) = ( shear / (1 - 2 * nu) ) * 2 * ( 1-nu ) ;
    
    ConsMat (4-1,4-1 ) = shear ;
    ConsMat (5-1,5-1 ) = shear ;
    ConsMat (6-1,6-1 ) = shear ;
  }
}
// ==============================================================================




// ======================================================================
// BgrandeMats
// ======================================================================
mat BgrandeMats ( mat deriv , mat F ){

  mat matBgrande = zeros<mat>(6, 12);
  
  for (int k=1; k<=4; k++){

    for (int i=1; i<=3 ; i++){

      for (int j=1; j<=3; j++){
        matBgrande ( i-1 , (k-1)*3 + j -1  ) = deriv(i-1,k-1) * F(j-1,i-1) ;
      }
    }          

    for (int j=1; j<=3; j++){
      matBgrande ( 4-1 , (k-1)*3 + j-1 ) = deriv(2-1,k-1) * F(j-1,3-1) + deriv(3-1,k-1) * F(j-1,2-1) ;
      matBgrande ( 5-1 , (k-1)*3 + j-1 ) = deriv(1-1,k-1) * F(j-1,3-1) + deriv(3-1,k-1) * F(j-1,1-1) ;
      matBgrande ( 6-1 , (k-1)*3 + j-1 ) = deriv(1-1,k-1) * F(j-1,2-1) + deriv(2-1,k-1) * F(j-1,1-1) ;
    }
  } // for j
  return matBgrande;
}
// ======================================================================


// ======================================================================
vec mat2voigt( mat Tensor, double factor ){
    
  vec v = zeros<vec>(6);
    
  v(1-1) = Tensor(1-1,1-1) ;
  v(2-1) = Tensor(2-1,2-1) ;
  v(3-1) = Tensor(3-1,3-1) ;
  v(4-1) = Tensor(2-1,3-1)*factor ;
  v(5-1) = Tensor(1-1,3-1)*factor ;
  v(6-1) = Tensor(1-1,2-1)*factor ;
  
  return v;
}
// ======================================================================



// ======================================================================
//
//~ // ======================================================================
//~ mat constTensor ( vec hyperElasParamsVec, mat Egreen ){

  //~ mat ConsMat = zeros<mat> (6,6);
  
  //~ double young = hyperElasParamsVec(0)       ;
  //~ double nu = hyperElasParamsVec(1)          ;
  //~ double shear   = young / ( 2.0 * (1+ nu) ) ;


  //~ return ConsMat;
//~ }
//~ // ======================================================================





// =====================================================================
//  elementTetraSVKSolidInternLoadsTangMat
// =====================================================================
void elementTetraSolid( vec elemCoords, vec elemDisps, vec elemConstitutiveParams, \
    int paramOut, int consMatFlag, double elemrho, vec & Finte, mat & KTe ){
  
    // reset element forces
  Finte.zeros();  KTe.zeros();

  //~ %~ [ deriv , vol ] =  DerivFun( tetCoordMat ) ;
  //~ %~ tetVol = vol ;
  //~ %~ BMat = BMats ( deriv ) ;

  mat eleCoordMat = reshape( elemCoords, 3, 4 ) ;
  mat eleDispsMat = reshape( elemDisps , 3, 4 ) ;
  //~ mat eleCoordSpa = tetCoordMat + eleDispsMat ;

  // matriz de derivadas de fun forma respecto a coordenadas isoparametricas
  double xi = 0.25 ;  double wi = 1.0 / 6.0  ;
  mat deriv = shapeFunsDeriv( xi, xi , xi ) ;

  // jacobiano que relaciona coordenadas materiales con isoparametricas
  mat jacobianmat = eleCoordMat * deriv  ;

  double vol = det( jacobianmat ) * wi ;

  if (vol<0){
    cout << "Element with negative volume" << vol << " check connectivity." << endl;
    exit(0);
  }
  
  //~ cout << vol << endl;

  // displacement gradient
  mat funder = deriv * inv(jacobianmat) ;
  mat H = eleDispsMat * funder ;
  mat F = H + eye(3,3) ;
  mat Egreen = 0.5 * ( H + H.t() + H.t() * H ) ;

  mat S, ConsMat(6,6);
  
  if (elemConstitutiveParams(1-1) == 2){ // Saint-Venant-Kirchhoff compressible solid
    cosseratSVK( elemConstitutiveParams(span(2-1,3-1)), Egreen, consMatFlag, S, ConsMat );
  //~ }else if (elemConstitutiveParams(1-1) == 3){ // Neo-Hookean Compressible
    //~ [ S, ConsMat ] = cosseratNH ( elemConstitutiveParams(2:3), Egreen, consMatFlag ) ;
  }

  mat matBgrande = BgrandeMats ( funder.t() , F ) ;

  vec Svoigt = mat2voigt( S, 1 ) ;

  Finte  = matBgrande.t() * Svoigt * vol ;

  if (paramOut == 2){
    mat Kml        = matBgrande.t() * ConsMat * matBgrande * vol ;
    mat matauxgeom = funder * S * funder.t()  * vol ;
    mat Kgl        = zeros<mat>(12,12) ;
    for   (int i=1; i<=4; i++){
      for (int j=1; j<=4; j++){
        Kgl( (i-1)*3+1-1 , (j-1)*3+1-1 ) = matauxgeom( i-1, j-1 ) ;
        Kgl( (i-1)*3+2-1 , (j-1)*3+2-1 ) = matauxgeom( i-1, j-1 ) ;
        Kgl( (i-1)*3+3-1 , (j-1)*3+3-1 ) = matauxgeom( i-1, j-1 ) ;
      }
    }
    KTe = Kml + Kgl ;
  }

}
// =====================================================================






// =====================================================================
//
// =====================================================================
ivec elementTypeInfo( int elemType ){
  ivec out(2);  //~ return numNodes, dofsStep
  if ( elemType == 1){
    out={1,1};
  }else if ( elemType == 4){ // tetrahedron
    out={4,2};
  }
  return out ;
}
// =============================================================================






// =====================================================================
// assembler
// =====================================================================
void assembler( imat conec, mat crossSecsParamsMat, mat coordsElemsMat, \
  mat materialsParamsMat, sp_mat KS, vec Ut, int paramOut, vec Udott, \
  vec Udotdott, double nodalDispDamping, uint solutionMethod, uvec neumdofs, \
  mat elementsParamsMat, field<vec> & fs, field<sp_mat> & ks ){
  
  // ====================================================================
  //  --- 1 declarations ---
  // ====================================================================

  // -----------------------------------------------
  int nElems  = conec.n_rows;
  int nNodes  = numel( Ut ) / 6 ;
  int typeElem, numNodes ; // numNodes is the number of nodes per element
  double elemrho;
  
  vec Fint( nNodes*6, fill::zeros ) ;
  vec Fmas( nNodes*6, fill::zeros ) ;
  vec Fvis( nNodes*6, fill::zeros ) ;
  
  if (paramOut == 1){
    //~ // -------  residual forces vector ------------------------------------
    //~ // --- creates Fint vector ---
   }
   
  int indTotal = 0 ;
  
  vec Finte(12) ; 
  mat KTe(12,12);
  
  ivec nodeselem, dofselem, auxel;
  
  //~ ivec indsIKT( nElems*24*24, fill::zeros ) ;
  //~ ivec indsJKT( nElems*24*24, fill::zeros ) ;
  
  umat locsKT( 2, nElems*24*24, fill::zeros );
  vec  valsKT(    nElems*24*24, fill::zeros ) ;


  uvec dofselemRed( 4*6/2 ) ;
  vec elemCoords( 4*6/2 );
  vec elemDisps( 4*6/2);

  for( int elem = 1; elem <= nElems; elem++){

    // material parameters
    vec elemMaterialParams     = materialsParamsMat.row( conec( elem-1, 5-1 )-1 ).t() ;
    elemrho                    = elemMaterialParams( 1 - 1 ) ;
    vec elemConstitutiveParams = elemMaterialParams.rows( 2-1 , elemMaterialParams.n_elem-1 ) ;

    // element parameters
    vec elemElementParams      = elementsParamsMat.row( conec( elem-1, 6-1 )-1 ).t()  ;
    
    typeElem = elemElementParams(1-1) ;
    
    auxel = elementTypeInfo ( typeElem ) ;
    numNodes = auxel( 0 ) ;
    
    // obtains nodes and dofs of element
    nodeselem   = conec( elem-1, span(1-1,numNodes-1) ).t()      ;
    dofselem    = nodes2dofs( nodeselem , 6 )  ;

    for ( int ind=1; ind <= (4*3); ind++ ){
      dofselemRed( ind-1) = dofselem ( 2*(ind-1)+1-1 ) ;
      elemCoords( ind-1)  = coordsElemsMat( elem-1, 2*(ind-1)+1-1 ) ;
    }
    
    elemDisps = Ut.elem( dofselemRed-1) ;
    
    if ( typeElem == 4){
      elementTetraSolid( elemCoords, elemDisps, elemConstitutiveParams, paramOut, elemElementParams(2-1), elemrho, Finte, KTe ) ;
    }

    // assembly Fint
    for (int indi=1; indi<= 12; indi++){
      Fint( dofselemRed( indi-1 )-1 ) = Fint( dofselemRed( indi-1 )-1 ) + Finte( indi-1 ) ;
    }
    
    if (paramOut == 2){  

      uvec posi, posj;
      
      for ( int indi=1; indi<=12; indi++){
        for ( int indj=1; indj<=12; indj++){
	  	    
	  posi = find ( neumdofs == dofselemRed( indi-1 ) ) ;
	  posj = find ( neumdofs == dofselemRed( indj-1 ) ) ;
	  
	  if ( (posi.n_elem == 1) && ( posj.n_elem == 1 ) ){
	    indTotal++;
	    //~ locsKT ( 0, indTotal-1 ) = neumdofs( posi(0) ) ;
	    //~ locsKT ( 1, indTotal-1 ) = neumdofs( posj(0) ) ;
	    locsKT ( 0, indTotal-1 ) = posi(0) ;
	    locsKT ( 1, indTotal-1 ) = posj(0) ;
	    valsKT ( indTotal-1 )    = KTe( indi-1, indj-1 ) ;
	  } // if dof is in neumdofs
        } // for rows     
      } // for cols
    } // if paramOut 2
  } // for elements
  // -------------------------------------------------------------------  

  fs(0,0) = Fint ;   fs(1,0) = Fvis ;   fs(2,0) = Fmas ;

  //~ bool add_values = true;
  if (paramOut == 2){
    sp_mat aux( true, locsKT.cols(0,indTotal-1), valsKT(span(0,indTotal-1)), neumdofs.n_elem, neumdofs.n_elem );
    ks(0,0) = aux ;
  }
}
// =============================================================================




// =============================================================================
// --- extractMethodParams ---
// =============================================================================
void extractMethodParams( vec numericalMethodParams, uint & solutionMethod, \
                          double & stopTolDeltau, double & stopTolForces,  \
                          uint & stopTolIts, double & targetLoadFactr, \
                          uint & nLoadSteps, double & incremArcLen, \
                          double & deltaT, double & deltaNW, double & AlphaNW, \
                          double & alphaHHT, double & finalTime ){
  
  solutionMethod   = numericalMethodParams(1-1) ;
  
  if (solutionMethod == 1){

    // ----- resolution method params -----
    stopTolDeltau    = numericalMethodParams(2-1) ;
    stopTolForces    = numericalMethodParams(3-1) ;
    stopTolIts       = numericalMethodParams(4-1) ;
    targetLoadFactr  = numericalMethodParams(5-1) ;
    nLoadSteps       = numericalMethodParams(6-1) ;
  
    incremArcLen     = 0 ;
   
    deltaT = targetLoadFactr / double( nLoadSteps) ;
    
    finalTime = targetLoadFactr ;
    
    deltaNW =  0; AlphaNW = 0 ; alphaHHT = 0 ;
  }
}
// =============================================================================




// =============================================================================
// --- computeFext ---
// =============================================================================
void computeFext( vec constantFext, vec variableFext, double nextLoadFactor, \
  string userLoadsFilename, \
  vec & FextG ){
  
  FextG  = variableFext * nextLoadFactor + constantFext  ;//+ FextUser  ;
}
// =============================================================================







// =============================================================================
// --- computeRHS ---
// =============================================================================
void computeRHS( imat conec, mat crossSecsParamsMat, mat coordsElemsMat, \
    mat materialsParamsMat, sp_mat KS, vec constantFext, vec variableFext, \
    string userLoadsFilename, double currLoadFactor, \
    double nextLoadFactor, vec numericalMethodParams, uvec neumdofs, \
    double nodalDispDamping, vec Ut, vec Udott, vec Udotdott, vec Utp1, \
    vec Udottp1, vec Udotdottp1, mat elementsParamsMat, \
    vec & systemDeltauRHS, vec & FextG ){
  
  uint solutionMethod, stopTolIts, nLoadSteps ;
  double stopTolDeltau, stopTolForces, incremArcLen, targetLoadFactr, \
    deltaT, deltaNW, AlphaNW, alphaHHT, finalTime ;
			  
  extractMethodParams  ( numericalMethodParams, solutionMethod, \
                          stopTolDeltau, stopTolForces, stopTolIts, targetLoadFactr, \
                          nLoadSteps, incremArcLen, deltaT, deltaNW, AlphaNW, \
                          alphaHHT, finalTime );
			  
  field<vec>    fs(3,1) ;
  field<sp_mat> ks(3,1) ;
  
  assembler ( conec, crossSecsParamsMat, coordsElemsMat, materialsParamsMat, \
    KS, Utp1, 1, Udottp1, Udotdottp1, nodalDispDamping, solutionMethod, neumdofs, \
    elementsParamsMat, fs, ks ) ;

  vec Fint = fs(0,0) ;  vec Fvis = fs(1,0) ;   vec Fmas = fs(2,0) ;  

  computeFext( constantFext, variableFext, nextLoadFactor, userLoadsFilename, \
    FextG ) ;

  systemDeltauRHS = - ( Fint.elem( neumdofs-1 ) - FextG.elem( neumdofs-1 ) ) ;
 
}
// =============================================================================



// =============================================================================
//  updateTime
// =============================================================================
void updateTime( vec Ut, vec Udott, vec Udotdott, vec Utp1k, \
  vec numericalMethodParams, double currTime, \
  vec & Udottp1k, vec & Udotdottp1k, double & nextTime ){
      
  uint solutionMethod, stopTolIts, nLoadSteps;
  double stopTolDeltau, stopTolForces, targetLoadFactr, incremArcLen, deltaT, \
    deltaNW, AlphaNW, alphaHHT, finalTime;
  
  extractMethodParams( numericalMethodParams, solutionMethod, stopTolDeltau, \
                       stopTolForces, stopTolIts, targetLoadFactr, nLoadSteps, \
		       incremArcLen, deltaT, deltaNW, AlphaNW, alphaHHT, finalTime );

  nextTime    = currTime + deltaT ;
  Udotdottp1k = Udotdott          ;
  Udottp1k    = Udott             ;
}
// =============================================================================









// =============================================================================
vec updateUiter( vec Utp1k, vec deltaured, uvec neumdofs, uint solutionMethod ){
  for ( uint i=1; i<= neumdofs.n_elem; i++){
    Utp1k( neumdofs(i-1)-1) = Utp1k( neumdofs(i-1)-1) + deltaured( i-1) ;
  }
  return Utp1k ;  
}
// =============================================================================






// =============================================================================
void computeDeltaU ( sp_mat systemDeltauMatrix, vec systemDeltauRHS, uint dispIter, vec numericalMethodParams, double nextLoadFactor, vec currDeltau, vec & deltaured ){
  
  deltaured = spsolve( systemDeltauMatrix, systemDeltauRHS );
}
// =============================================================================


// =============================================================================
//  compute matrix
// =============================================================================
sp_mat computeMatrix( imat conec, mat crossSecsParamsMat, mat coordsElemsMat, \
  mat materialsParamsMat, sp_mat KS, vec Uk, uvec neumdofs, vec numericalMethodParams, \
  double nodalDispDamping, vec Udott, vec Udotdott, mat elementsParamsMat ){

  uint solutionMethod, nLoadSteps, stopTolIts ;
  double stopTolDeltau, stopTolForces, targetLoadFactr, \
    incremArcLen, deltaT, deltaNW, AlphaNW, alphaHHT, finalTime ;  

  extractMethodParams( numericalMethodParams, solutionMethod, stopTolDeltau, \
    stopTolForces, stopTolIts, targetLoadFactr, nLoadSteps, incremArcLen, \
    deltaT, deltaNW, AlphaNW, alphaHHT, finalTime );
    
  field<vec>    fs(3,1) ;
  field<sp_mat> ks(3,1) ;

  // computes static tangent matrix
  assembler( conec, crossSecsParamsMat, coordsElemsMat, materialsParamsMat, \
    KS, Uk, 2, Udott, Udotdott, nodalDispDamping, solutionMethod, neumdofs, \
    elementsParamsMat, fs, ks );
    
  return ks(0,0) ;
}
// =============================================================================






void  convergenceTest( vec numericalMethodParams, vec redFext, \
  vec redDeltaU, vec redUk, uint dispIters, vec systemDeltauRHS, \
  bool & booleanConverged, uint & stopCritPar, double & deltaErrLoad ){

  uint solutionMethod, nLoadSteps, stopTolIts ;
  double stopTolDeltau, stopTolForces, targetLoadFactr, \
    incremArcLen, deltaT, deltaNW, AlphaNW, alphaHHT, finalTime ;  

  extractMethodParams( numericalMethodParams, solutionMethod, stopTolDeltau, \
    stopTolForces, stopTolIts, targetLoadFactr, nLoadSteps, incremArcLen, \
    deltaT, deltaNW, AlphaNW, alphaHHT, finalTime );

  double normaUk     = norm( redUk )               ;
  double normadeltau = norm( redDeltaU         )   ;
  // deltaErrLoad  = norm( redFint - redFext - redFinet )   ;

  deltaErrLoad    = norm( systemDeltauRHS )     ;
  double normFext = norm( redFext         )     ;
  
  bool logicDispStop = ( normadeltau  < ( normaUk  * stopTolDeltau ) )  ;
  bool logicForcStop = ( deltaErrLoad < ( (normFext+(normFext < stopTolForces)) * stopTolForces ) )  && ( deltaErrLoad > 0 ) ;
   
  if ( logicForcStop ){
    stopCritPar = 1 ;      booleanConverged = 1 ;
  }else if ( logicDispStop ){
    stopCritPar = 2 ;      booleanConverged = 1 ;
  }else if ( dispIters >= stopTolIts ){
    stopCritPar = 3 ;      booleanConverged = 1 ;
  }else{
    stopCritPar = 0 ;      booleanConverged = 0 ; 
  }
}
// =============================================================================





// =============================================================================
//  printSolverOutput
// =============================================================================
void printSolverOutput( string outputDir, string problemName, uint timeIndex, vec params){

//{ 1, dispIters, deltaErrLoad, norm(deltaured)

  
  string incrementsResultsFilename = outputDir + problemName + "_incrementsOutput.tex" ;

//~ incrementsNormalForce 		= [ outputdir  problemName '_incrementsNormalForceOutput.tex' ] ;
//~ %~ incrementsTimePerformance = [ outputdir  problemName '_timePerformanceOutput.tex' ] ;

//~ headerIncrements  = [ '$\\#t$ & $ \\lambda(t)$ & its & $\\| RHS \\|$ & $\\| \\Delta u \\|$ & flagExit ' ...
                      //~ ' & npos & nneg  \\\\ \\hline \n \\endhead \n'] ;
//~ %
//~ headerNormalForce = [ 'timeInd & t & $ \\lambda(t)$ & $N_{max}$ & $N_{min}$ \\\\ \\toprule \n'] ;

//~ timeStepEndLine   = [ '\\hdashline\n' ...
                      //~ '%4i & %9.2e & %4i &           &           & %2i & %3i & %3i \\\\ \n' ] ;
//~ %
//~ timeStepIterLine  = [ '     &           & %4i & %9.2e & %9.2e &    &     &     \\\\ \n' ] ;

//~ if timeIndex == 1 && lineData(1)~=1

  //~ % opens and rewrites files
  //~ fileIncrements = fopen( incrementsResultsFilename ,'w');
  //~ fileNormalForce = fopen( incrementsNormalForce, 'w' ) ;

  //~ % write headers
  //~ fprintf( fileIncrements, headerIncrements );
  //~ fprintf(fileNormalForce, headerNormalForce  );
  
//~ else
  //~ fileIncrements 			= fopen( incrementsResultsFilename ,'a' ) ;
  //~ fileNormalForce 		= fopen( incrementsNormalForce, 'a' ) 		;
  //~ %~ fileTimePerformance = fopen( incrementsTimePerformance, 'a' ) ;
//~ end

  //~ %~ fileTimePerformance = fopen( incrementsTimePerformance, 'w' ) ;
  //~ %~ fprintf(fileTimePerformance, [ 'timeInd & t & Solver time (s) & Stores time (s)  \\\\ \\toprule \n'] );
  

//~ %~ timeIndex, currTime, currLoadFactor,  auxIO.itersPerTime, max( max( abs( modelCurrState.Strainst) )*100 ) , ...
  //~ %~ factor_crit , nKeigpos, nKeigneg

//~ %~ stop  
//~ % latex table output
//~ if lineData(1) == 1
  //~ fprintf( fileIncrements, timeStepIterLine, lineData(2), lineData(3), lineData(4) ) ;
  //~ %~ fileIncrements
  //~ %~ lineData
  //~ %~ fclose( fileIncrements )
  //~ %~ timeIndex, currLoadFactor,  auxIO.itersPerTime, max( max( abs( modelCurrState.Strainst) )*100 ) , ...
  //~ %~ factor_crit , nKeigpos, nKeigneg )

//~ %~ fprintf(fileNormalForce, [ ' %4i & %12.3e & %12.3e  & %12.3e & %12.3e \\\\\n' ], ...
  //~ %~ timeIndex, currTime, currLoadFactor, max(currentNormalForces), min(currentNormalForces)  )

//~ %~ fprintf(fileTimePerformance, [' %4i & %12.3e & %5.3e & %5.3e \\\\\n' ], ...
	//~ %~ timeIndex, currTime, tCallSolver, tStores)     
//~ %~ % -----------------------------------

//~ %~ if max( abs( Strainst) ) > 0.05,
  //~ %~ fprintf('WARNING: at timeStep %5i, elements with strain level %4.1f%%!\n', timeIndex, max( abs( Strainst) )*100 ),
  
//~ elseif lineData(1) == 2

//~ %~ printSolverOutput( outputDir, problemName, timeIndex, [ 2 nextLoadFactor dispIter deltaErrLoad norm(deltaured) stopCritPar nKeigpos nKeigneg ] ) ;
  //~ fprintf( fileIncrements, timeStepEndLine, timeIndex,  lineData(2), lineData(3), lineData(4), lineData(5), lineData(6) );
  
//~ end

//~ % close files
//~ fclose(fileIncrements);
//~ fclose(fileNormalForce);
//~ %~ fclose(fileTimePerformance);

}
// =============================================================================




// =============================================================================
//  main
// =============================================================================
int main(){
  
  cout << "\n=============================" << endl;
  cout << "=== C++ timeStepIteration ===" << endl;

  // ---------------------------------------------------------------------------
  // --------                       reading                          -----------
  // ---------------------------------------------------------------------------
  //~ cout << "  reading inputs..." ;
  
  // declarations of variables read
  mat conecDouble                        ;
  vec numericalMethodParams              ;
  sp_mat systemDeltauMatrix              ;
  sp_mat KS                              ;
  vec U, Udot, Udotdot, Fint, Fmas, Fvis ;
  vec systemDeltauRHS                    ;
  vec FextG, constantFext, variableFext  ;
  string userLoadsFilename               ;
  vec scalarParams                       ;

  scalarParams.load("scalarParams.dat")  ;

  double currLoadFactor   = scalarParams(0) , \
         nextLoadFactor   = scalarParams(1) , \
	 nodalDispDamping = scalarParams(2) , \
	 currTime         = scalarParams(3) ;
  
  uint timeIndex = scalarParams(4);
  string outputDir, problemName;
  
  double nextTime ;
  double deltaErrLoad ;

  vec deltaured ;
  vec Udottp1k, Udotdottp1k ;

  ifstream ifile                         ;
  
  // reading
  conecDouble.load("Conec.dat", raw_ascii);
  imat conec = conv_to<imat>::from(conecDouble);
  
  numericalMethodParams.load("numericalMethodParams.dat");

  systemDeltauMatrix.load("systemDeltauMatrix.dat", coord_ascii);
  systemDeltauMatrix = systemDeltauMatrix.tail_rows(systemDeltauMatrix.n_rows-1);
  systemDeltauMatrix = systemDeltauMatrix.tail_cols(systemDeltauMatrix.n_cols-1);
  
  KS.load("KS.dat", coord_ascii);
  if ( KS.n_rows > 0 ){
    KS = KS.tail_rows(KS.n_rows-1);
    KS = KS.tail_cols(KS.n_cols-1);
  }
    
      U.load("U.dat"      );
  ifile.open("Udot.dat"   ); if(ifile){    Udot.load("Udot.dat"   );}else{ Udot.zeros   ( U.n_elem );}
  ifile.open("Udotdot.dat"); if(ifile){ Udotdot.load("Udotdot.dat");}else{ Udotdot.zeros( U.n_elem );}


  constantFext.load("constantFext.dat");
  variableFext.load("variableFext.dat");
  
  vec auxvec; auxvec.load("neumdofs.dat");
  uvec neumdofs = conv_to<uvec>::from( auxvec ) ;
  
  //~ cout << "variableFext: " << variableFext << endl;
  //~ cout << "neumdofs: " << neumdofs << endl;

  mat coordsElemsMat ;
  
  // MELCS parameters matrices
  mat materialsParamsMat, elementsParamsMat, crossSecsParamsMat;  

  materialsParamsMat.load("materialsParamsMat.dat") ;
  elementsParamsMat.load( "elementsParamsMat.dat" ) ;
  coordsElemsMat.load( "coordsElemsMat.dat" ) ;
  
  ifstream input("strings.txt");
  
  input >> outputDir;
  input >> problemName;
  // ---------------------------------------------------------------------------


  // ---------------------------------------------------------------------------
  // --------                       pre                              -----------
  // ---------------------------------------------------------------------------
  
  // declarations
  int nelems    = conec.n_rows ;  int ndofpnode = 6;
  uint solutionMethod, nLoadSteps, stopTolIts ;
  double stopTolDeltau, stopTolForces, targetLoadFactr, \
    incremArcLen, deltaT, deltaNW, AlphaNW, alphaHHT, finalTime ;  

  extractMethodParams( numericalMethodParams, solutionMethod, stopTolDeltau, \
    stopTolForces, stopTolIts, targetLoadFactr, nLoadSteps, incremArcLen, \
    deltaT, deltaNW, AlphaNW, alphaHHT, finalTime );
  // ---------------------------------------------------------------------------



  // ---------------------------------------------------------------------------
  // ----       iteration in displacements or load-displacements         -------
  // ---------------------------------------------------------------------------
  
  sp_mat KTtred = systemDeltauMatrix ;

  // assign disps and forces at time t
  vec Ut    = U    ;   vec Udott = Udot ;   vec Udotdott = Udotdot ;
  
  vec Utp1k = Ut   ; // initial guess displacements
  // initial guess velocities and accelerations

  updateTime( Ut, Udott, Udotdott, Utp1k, numericalMethodParams, \
    currTime, Udottp1k, Udotdottp1k, nextTime ) ;

  // --- compute RHS for initial guess ---
  computeRHS( conec, crossSecsParamsMat, coordsElemsMat, materialsParamsMat, KS, \
    constantFext, variableFext, userLoadsFilename, currLoadFactor, \
    nextLoadFactor, numericalMethodParams, neumdofs, nodalDispDamping, \
    Ut, Udott, Udotdott, Utp1k, Udottp1k, Udotdottp1k, elementsParamsMat, \
    systemDeltauRHS, FextG ) ;
  // ---------------------------------------------------

  bool booleanConverged = 0                          ;
  uint dispIters        = 0 ;
  uint stopCritPar      = 0 ;
  vec  currDeltau( neumdofs.n_elem , fill::zeros ) ;
  

  while (booleanConverged == 0){
    
    dispIters++;

    // --- solve system ---
    computeDeltaU ( systemDeltauMatrix, systemDeltauRHS, dispIters, numericalMethodParams, nextLoadFactor , currDeltau, deltaured ) ;
    // ---------------------------------------------------

    // --- updates: model variables and computes internal forces ---
    Utp1k = updateUiter( Utp1k, deltaured, neumdofs, solutionMethod ) ;
    // ---------------------------------------------------
  
    // --- update next time magnitudes ---
    updateTime( Ut, Udott, Udotdott, Utp1k, numericalMethodParams, currTime, Udottp1k, Udotdottp1k, nextTime );
    // ---------------------------------------------------
  
    // --- system matrix ---
    systemDeltauMatrix  = computeMatrix( conec, crossSecsParamsMat, coordsElemsMat, \
      materialsParamsMat, KS, Utp1k, neumdofs, numericalMethodParams, nodalDispDamping, Udott, Udotdott, elementsParamsMat );
    // ---------------------------------------------------


    // --- new rhs ---
    computeRHS( conec, crossSecsParamsMat, coordsElemsMat, materialsParamsMat, KS, \
      constantFext, variableFext, userLoadsFilename, currLoadFactor, \
      nextLoadFactor, numericalMethodParams, neumdofs, nodalDispDamping, \
      Ut, Udott, Udotdott, Utp1k, Udottp1k, Udotdottp1k, elementsParamsMat, \
      systemDeltauRHS, FextG ) ;
    // ---------------------------------------------------

    // --- check convergence ---
    convergenceTest( numericalMethodParams, FextG.elem( neumdofs-1 ), deltaured, Utp1k.elem( neumdofs-1 ), dispIters, systemDeltauRHS, booleanConverged, stopCritPar, deltaErrLoad ) ;
    // ---------------------------------------------------
  
    cout << "iter: " << dispIters <<  " | norma RHS: " << deltaErrLoad << " | norma delta u " << norm( deltaured) << endl;

  
    //~ % --- prints iteration info in file ---
    printSolverOutput( outputDir, problemName, timeIndex, { 1, dispIters, deltaErrLoad, norm(deltaured) } ) ;
    
  }
  // --------------------------------------------------------------------
  
  vec Utp1       = Utp1k ;
  vec Udottp1    = Udottp1k ;
  vec Udotdottp1 = Udotdottp1k ;
  
  // computes KTred at converged Uk
  //~ KTtp1red = systemDeltauMatrix ;


//~ indsIKT.save("indsIKT.dat", raw_ascii);
    //~ indsJKT.save("indsJKT.dat", raw_ascii);
    //~ valsIKT.save("valsIKT.dat", raw_ascii);
    
  systemDeltauMatrix.save("systemDeltauMatrixCpp.dat", coord_ascii );
    
  //~ Udottp1    = Udottp1k ;
  //~ Udotdottp1 = Udotdottp1k ;
  
  //~ % computes KTred at converged Uk
  //~ KTtp1red = systemDeltauMatrix ;
  
  vec auxOutValsVec = { nextTime, stopCritPar, dispIters, solutionMethod } ;
 
  Ut.save("Ut.dat", raw_ascii);
  Utp1.save("Utp1.dat", raw_ascii);
  Udottp1.save("Udottp1.dat", raw_ascii);
  Udotdottp1.save("Udotdottp1.dat", raw_ascii);
  
  auxOutValsVec.save("auxOutValsVec.dat", raw_ascii);
  
  //~ % --------------------------------------------------------------------
  
  //~ Stresstp1 = assembler ( Conec, crossSecsParamsMat, coordsElemsMat, materialsParamsMat, KS, Utp1, 3, Udottp1, Udotdottp1, nodalDispDamping, solutionMethod, elementsParamsMat ) ;
  
  
  //~ % %%%%%%%%%%%%%%%%
  //~ stabilityAnalysisFlag = stabilityAnalysisBoolean ;
  //~ % %%%%%%%%%%%%%%%%
  
  //~ if stabilityAnalysisFlag == 2
    //~ [ nKeigpos, nKeigneg, factorCrit ] = stabilityAnalysis ( KTtred, KTtp1red, currLoadFactor, nextLoadFactor ) ;
  //~ elseif stabilityAnalysisFlag == 1
    //~ [ nKeigpos, nKeigneg ] = stabilityAnalysis ( KTtred, KTtp1red, currLoadFactor, nextLoadFactor ) ;
    //~ factorCrit = 0;
  //~ else
    //~ nKeigpos = 0;  nKeigneg = 0; factorCrit = 0 ;
  //~ end
  
  
  
  //~ % prints iteration info in file
  //~ printSolverOutput( ...
    //~ outputDir, problemName, timeIndex+1, [ 2 nextLoadFactor dispIters stopCritPar nKeigpos nKeigneg ] ) ;
  
  
  //~ % --- stores next step values ---
  //~ U          = Utp1 ;
  //~ Udot       = Udottp1  ;
  //~ Udotdot    = Udotdottp1 ;
  //~ convDeltau = Utp1 - Ut ;
  //~ %
  //~ Stress     = Stresstp1 ;
  
  //~ timeIndex  = timeIndex + 1 ;
  
  //~ currTime   = nextTime ;
  //~ if solutionMethod == 2
    //~ currTime = nextLoadFactor ;
  //~ end
  
  //~ timeStepStopCrit = stopCritPar ;
  //~ timeStepIters = dispIters ;



  return 0;
}
// =============================================================================




//~ function [deltaured, nextLoadFactor ] = computeDeltaU ( systemDeltauMatrix, systemDeltauRHS, dispIter, redConvDeltau, numericalMethodParams, nextLoadFactor, currDeltau  )

  //~ [ solutionMethod, stopTolDeltau,   stopTolForces, ...
    //~ stopTolIts,     targetLoadFactr, nLoadSteps,    ...
    //~ incremArcLen, deltaT, deltaNW, AlphaNW, finalTime ] ...
        //~ = extractMethodParams( numericalMethodParams ) ;
  
  //~ convDeltau = redConvDeltau ;  

  //~ if solutionMethod == 2
  
    //~ aux = systemDeltauMatrix \ systemDeltauRHS ;
    
    //~ deltauast = aux(:,1) ;  deltaubar = aux(:,2) ;
    
    //~ if dispIter == 1
      //~ if norm(convDeltau)==0
        //~ deltalambda = targetLoadFactr / nLoadSteps ;
      //~ else
        //~ deltalambda = sign( convDeltau' * deltaubar ) * incremArcLen / sqrt( deltaubar' * deltaubar ) ;
      //~ end
      
    //~ else
      //~ ca =    deltaubar' * deltaubar ;
      //~ cb = 2*(currDeltau + deltauast)' * deltaubar ;
      //~ cc =   (currDeltau + deltauast)' * (currDeltau + deltauast) - incremArcLen^2 ; 
      //~ disc = cb^2 - 4 * ca * cc ;
      //~ if disc < 0
        //~ disc, error( 'negative discriminant'); 
      //~ end
      //~ sols = -cb/(2*ca) + sqrt(disc) / (2*ca)*[-1 +1]' ;
      
      //~ vals = [ ( currDeltau + deltauast + deltaubar * sols(1) )' * currDeltau   ;
               //~ ( currDeltau + deltauast + deltaubar * sols(2) )' * currDeltau ] ;
     
      //~ deltalambda = sols( find( vals == max(vals) ) ) ;
    //~ end
    
    //~ nextLoadFactor  = nextLoadFactor  + deltalambda(1) ;
    
    //~ deltaured = deltauast + deltalambda(1) * deltaubar ;
  
  
  //~ else   % incremental displacement
    //~ deltaured = systemDeltauMatrix \ systemDeltauRHS ;
  
  
  //~ end
// ======================================================================














  //~ varsInps.load("varsInps.dat");
  //~ coordsElemsMat.load("coordsElemsMat.dat");
  //~ hyperElasParamsMat.load("materialsParamsMat.dat");

  // --- processing ---  
  
  //~ int nnodes = ( varsInps.n_rows -1) / 6 ;
  //~ int paramOut = varsInps( varsInps.n_rows - 1 ) ;
  //~ vec Ut = varsInps( span(0,varsInps.n_rows - 2), span(0,0) ) ;

  //~ cout << "  paramOut: " << paramOut << endl;
  //~ cout << "  nnodes: " << nnodes << endl;
  // -------------------------------------------------------------------
  
  //~ cout << "Ut: " << Ut << endl;




//~ vec Fintt = Fint ;   vec Fmast = Fmas ;   vec Fvist    = Fvis    ;
   //~ Fint.load("Fint.dat"   );
  
//~ ifile.open("Utp1.dat"   ); if(ifile){ Utp1.load("Utp1.dat")      ;}else{ Fmas.zeros()   ;}
//~ ifile.open("Utp1.dat"   ); if(ifile){ Utp1.load("Utp1.dat")      ;}else{ Fmas.zeros()   ;}
  //~ ifile.open("Fvis.dat"   ); if(ifile){ Fvis.load("Fvis.dat")      ;}else{ Fvis.zeros()   ;}
