//
// Created by gs1010 on 27/04/20.
//

#ifndef MLPROJECT_SRC_OPTIMIZER_LBFGS_H_
#define MLPROJECT_SRC_OPTIMIZER_LBFGS_H_

#include <deque>
#include "optimizer.h"

class LBFGS : public Optimizer {
 private:
  //! The pair is defined as: {s_{i},y_{i}} where s_{i}= w_{k+1} - w_{k} and y_{k}= \nabla f_{k+1} - \nabla f_{k}.
  //! deque chose because we need to iterate forward and backward.
  //! The most recently curvature information are pushed in the front on the queue.
  //! The pop operation is done on the tail of the queue.
  std::vector<std::deque<std::pair<arma::mat, arma::mat>>>
      pastCurvatureLayer;
  double alpha_max;
  int maxStep;
  double c1;
  double c2;
  unsigned long storageSize;
  std::default_random_engine re;

  double lineSearchBacktracking(Network *currNet,
                                const double weightDecay, const double momentum);

  double lineSearch(Network *currNet,
                    const double weightDecay, const double momentum);

  double zoom(Network *currNet,
              const double weightDecay,
              const double momentum,
              double alphaLow,
              double alphaHi,
              const double phi0,
              const double initialSearchDirectionDotGradient);
  void computeLayersDirections(std::vector<Layer> &net);
  inline void searchDirection(arma::mat &&approxInvHessian,
                              arma::mat &&q,
                              arma::mat &&currentLayerDirection,
                              const size_t indexLayer);
  void secantEquationCondition(const size_t indexLayer);
  inline double computeDirectionDescent(Network *currNetwork);
  double quadraticInterpolation(double alphaLo, double phiAlphaLo, double searchDirectionDotGradientAlphaLo,
                                double alphaHi,
                                double phiAlphaHi);
  double cubicInterpolation(double alphaLo,
                            double phiAlphaLo,
                            double searchDirectionDotGradientAlphaLo,
                            double alphaHi,
                            double phiAlphaHi,
                            double searchDirectionDotGradientAlphaHi);

 public:
  ~LBFGS() override = default;
  LBFGS(const int nLayer, const int storageSize = 15, const int seed = 0);
  void OptimizeBackward(Network *currNet,
                        const arma::mat &&partialDerivativeOutput,
                        const double momentum = 0.0) override;
  void OptimizeUpdateWeight(Network *network, const double learningRate,
                            const double weightDecay, const double momentum) override;
  void SetParameters(int storageSize_ = 15,
                     int maxStep_ = 100,
                     double alpha_max_ = 1,
                     double c1_ = 0.001,
                     double c2_ = 0.9);
};

#endif //MLPROJECT_SRC_OPTIMIZER_LBFGS_H_
