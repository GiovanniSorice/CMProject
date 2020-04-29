//
// Created by gs1010 on 27/04/20.
//

#include "LBFGS.h"
/***
 *
 * @param currNetwork
 * @param partialDerivativeOutput
 */
void LBFGS::OptimizeBackward(Network *currNetwork, const arma::mat &&partialDerivativeOutput) {
  std::vector<Layer> &net = currNetwork->GetNet();
  auto currentLayer = net.rbegin();
  currentLayer->OutputLayerGradient(std::move(partialDerivativeOutput));
  arma::mat currentGradientWeight;
  currentLayer->RetroPropagationError(std::move(currentGradientWeight));
  currentLayer++;
  // Iterate from the precedent Layer of the tail to the head
  for (; currentLayer != net.rend(); currentLayer++) {
    //! Add "old" gradient (\nabla f_{k-1})
    arma::mat oldGradient = currentLayer->GetGradientWeight();
    currentLayer->Gradient(std::move(currentGradientWeight));
    //! Save previous y_{k-1}= \nabla f_{k} - \nabla f_{k-1}
    if (pastCurvature.size()) {
      pastCurvature.begin()->second = currentLayer->GetGradientWeight() - oldGradient;
    }

    currentLayer->RetroPropagationError(std::move(currentGradientWeight));
  }
  computeLayersDirections(net);
}
/***
 *
 * @param currNet
 * @return
 */
double LBFGS::lineSearch(Network *currNet) {
  return 0;
}
/***
 * Following Algorithm 7.4 in chapter 7 of Numerical Optimization Jorge Nocedal Stephen J. Wright
 * @param net
 */
void LBFGS::computeLayersDirections(std::vector<Layer> &net) {
  for (Layer &currentLayer : net) {
    const arma::uword sizeGradient = currentLayer.GetGradientWeight().n_elem;
    //Compute initial hessian
    //! H_{k}^{0} = \gamma_{k}*I
    //! where \gamma_{k} = \frac{s^T_{k-1}y_{k-1}}{y^T_{k-1}y_{k-1}}
    arma::mat approxInvHessian = arma::eye(sizeGradient, sizeGradient);
    if (pastCurvature.size()) {
      arma::mat gamma = (pastCurvature.front().first.t() * pastCurvature.front().second)
          / (pastCurvature.front().second.t() * pastCurvature.front().second);
      approxInvHessian = gamma * approxInvHessian;
    }
    arma::mat q = currentLayer.GetGradientWeight();
    arma::mat currentLayerDirection;
    searchDirection(std::move(approxInvHessian), std::move(q), std::move(currentLayerDirection));
    currentLayer.SetDirection(std::move(currentLayerDirection));
  }

}
/***
 *
 * @param approxInvHessian
 * @param q
 * @param r
 */
void LBFGS::searchDirection(arma::mat &&approxInvHessian, arma::mat &&q, arma::mat &&r) {
  //! Saving alpha and rho results to avoid redundant computation
  std::vector<arma::mat> alpha;
  alpha.reserve(pastCurvature.size());
  std::vector<arma::mat> rho;
  rho.reserve(pastCurvature.size());
  auto currentReverseRho = rho.rbegin();
  auto currentReverseAlpha = alpha.rbegin();
  //! Iterate from head to tail ( from the earliest curvature information to the latest)
  for (std::pair<arma::mat, arma::mat> &currentPastCurvature : pastCurvature) {
    //! \rho= \frac{1}{y_{k}^{T}*s_{k}}
    *currentReverseRho = 1 / (currentPastCurvature.second.t() * currentPastCurvature.first);
    *currentReverseAlpha = *currentReverseRho * currentPastCurvature.first.t() * q;
    q = q - *currentReverseAlpha * currentPastCurvature.second;
    currentReverseAlpha++;
    currentReverseRho++;
  }
  r = approxInvHessian * q;
  arma::mat beta;
  auto currentRho = rho.begin();
  auto currentAlpha = alpha.begin();
  //! Iterate from tail to head ( from the latest curvature information to the earliest)
  for (auto currentPastCurvature = pastCurvature.rbegin(); currentPastCurvature != pastCurvature.rend();
       ++currentPastCurvature) {
    beta = *currentRho * currentPastCurvature->second.t() * r;
    r = r + currentPastCurvature->first * (*currentAlpha - beta);
    currentAlpha++;
    currentRho++;
  }
}
/***
 *
 * @param currNetwork
 */
void LBFGS::OptimizeUpdateWeight(Network *currNetwork, const double learningRate,
                                 const double weightDecay, const double momentum) {
  std::vector<Layer> &net = currNetwork->GetNet();

  for (Layer &currentLayer : net) {
    arma::mat oldWeight = currentLayer.GetWeight();
    currentLayer.AdjustWeight(learningRate, weightDecay, momentum);
    if (pastCurvature.size() == storageSize) {
      pastCurvature.pop_back();
    }
    //! Save s_{k}= w_{k+1} - w_{k}
    pastCurvature.push_front(std::make_pair(currentLayer.GetGradientWeight() - oldWeight, arma::mat(0)));
  }

}
LBFGS::LBFGS() : storageSize(5) {}
