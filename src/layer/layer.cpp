//
// Created by checco on 26/11/19.
//

#include "layer.h"
#include "../activationFunction/linearFunction.h"
#include "../activationFunction/logisticFunction.h"
#include "../activationFunction/reluFunction.h"
#include "../activationFunction/tanhFunction.h"
const arma::mat &Layer::GetWeight() const {
  return weight;
}
const arma::mat &Layer::GetBias() const {
  return bias;
}
const arma::mat &Layer::GetDelta() const {
  return deltaWeight;
}
const arma::mat &Layer::GetGradientWeight() const {
  return gradientWeight;
}
const arma::mat &Layer::GetInputParameter() const {
  return inputParameter;
}
const arma::mat &Layer::GetOutputParameter() const {
  return outputParameter;
}
const arma::mat &Layer::GetDeltaBias() const {
  return deltaBias;
}

Layer::Layer(const int inSize, const int outSize, const std::string activationFunctionString)
    : inSize(inSize),
      outSize(outSize),
      deltaWeight(arma::zeros(outSize, inSize)),
      deltaBias(arma::zeros(outSize, 1)) {

  if (activationFunctionString == "linearFunction") {
    activationFunction = new LinearFunction();
  }

  if (activationFunctionString == "logisticFunction") {
    activationFunction = new LogisticFunction();
  }

  if (activationFunctionString == "reluFunction") {
    activationFunction = new ReluFunction();
  }

  if (activationFunctionString == "tanhFunction") {
    activationFunction = new TanhFunction();
  }

  if (activationFunction == nullptr) {
    std::cout << activationFunctionString << " activationFunction not valid!" << std::endl;
    throw "activationFunction not valid!";
  }
}

/** Given the activated vector of the previous layer compute the forward pass
 *
 *  @param input Previous activated vector
 *  @param output Forwarded vector computed through weight and bias of the current layer
 * */
void Layer::Forward(const arma::mat &&input, arma::mat &&output, const double nesterovMomentum) {
  output = (weight + nesterovMomentum * deltaWeight) * input;
  output.each_col() += (bias + nesterovMomentum * deltaBias);

}

/** Compute the gradient of the output layer
 *
 *  @param partialDerivativeOutput Partial derivative of the output neuron
 * */
void Layer::OutputLayerGradient(const arma::mat &&partialDerivativeOutput) {
  arma::mat firstDerivativeActivation;
  activationFunction->Derive(std::move(outputParameter), std::move(firstDerivativeActivation));
  gradient = partialDerivativeOutput % firstDerivativeActivation;
  // Using gradient to avoid redundant computation
  gradientWeight = gradient * inputParameter.t();
}
int Layer::GetInSize() const {
  return inSize;
}
int Layer::GetOutSize() const {
  return outSize;

}

//! Ricorda che se vuoi avere run ripetibili, devi usare arma_rng::set_seed(value) al posto di arma::arma_rng::set_seed_random()
/***/
void Layer::Init(const double upperBound, const double lowerBound) {
  arma::arma_rng::set_seed(350);
  //arma::arma_rng::set_seed_random();
  weight = lowerBound + arma::randu<arma::mat>(outSize, inSize) * (upperBound - lowerBound);
  bias = lowerBound + arma::randu<arma::mat>(outSize, 1) * (upperBound - lowerBound);

}
void Layer::Activate(const arma::mat &input, arma::mat &&output) {
  activationFunction->Compute(input, std::move(output));
}
void Layer::SaveOutputParameter(const arma::mat &input) {
  outputParameter = input;
}
void Layer::SaveInputParameter(const arma::mat &input) {
  inputParameter = input;
}
/***/
void Layer::Gradient(const arma::mat &&summationGradientWeight) {

  arma::mat firstDerivativeActivation;
  activationFunction->Derive(std::move(outputParameter), std::move(firstDerivativeActivation));
  gradient = summationGradientWeight % firstDerivativeActivation;
  // Using gradient to avoid redundant computation
  gradientWeight = gradient * inputParameter.t();

}

/***/
void Layer::AdjustWeight(const double learningRate, const double weightDecay, const double momentum) {
  /*weight.print("weight pre");
  direction.print("direction");
   */
  weight = weight + momentum * deltaWeight - learningRate * direction
      - 2 * weightDecay * weight;
  /*weight.print("weight post");
  bias.print("bias pre");
   */
  bias = bias + momentum * deltaBias - learningRate * arma::mean(gradient, 1);
  //bias.print("bias post");

  deltaWeight = momentum * deltaWeight - learningRate * direction;
  deltaBias = momentum * deltaBias - learningRate * arma::mean(gradient, 1);
}

/**
 * Return a raw vector contains all the summed weight multiplied by the layer gradient
 */
void Layer::RetroPropagationError(arma::mat &&retroPropagatedError, const double nesterovMomentum) {
  retroPropagatedError = (weight + nesterovMomentum * deltaWeight).t() * gradient;
}

/**
 * Clear the internal variable of the layer (without delete the activationFunction)
 */
void Layer::Clear() {
  weight = arma::zeros(weight.n_rows, weight.n_cols);
  bias = arma::zeros(bias.n_rows, bias.n_cols);
  deltaWeight = arma::zeros(deltaWeight.n_rows, deltaWeight.n_cols);
  deltaBias = arma::zeros(deltaBias.n_rows, deltaBias.n_cols);
  gradientWeight = arma::zeros(gradientWeight.n_rows, gradientWeight.n_cols);
  gradient = arma::zeros(gradient.n_rows, gradient.n_cols);
  inputParameter = arma::zeros(inputParameter.n_rows, inputParameter.n_cols);
  outputParameter = arma::zeros(outputParameter.n_rows, outputParameter.n_cols);
}
/***
 *
 * @param optimizerComputedDirection
 */
void Layer::SetDirection(const arma::mat &&optimizerComputedDirection) {
  direction = optimizerComputedDirection;
}
