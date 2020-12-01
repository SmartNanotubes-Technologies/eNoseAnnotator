#include "leastsquaresfitter.h"
#include "vector"
#include <cmath>
#include <QtCore>

QMap<QString, LeastSquaresFitter::Type> LeastSquaresFitter::typeMap {{"Exposition", LeastSquaresFitter::Type::SUPERPOS}};


LeastSquaresFitter::LeastSquaresFitter()
{
    params = 1;
}

double LeastSquaresFitter::model(double input) const
{
    input_vector input_vector;
    input_vector(0) = input;

    return model(input_vector, params);
}

void LeastSquaresFitter::solve(const std::vector<std::pair<double, double> > &samples, int nIterations, double limitFactor)
{
    qDebug() << "solve";
    // find y_max
    double y_max = 0.;
    for (auto pair : samples)
    {
        if (pair.second > y_max)
            y_max = pair.second;
    }

    // prepare sample_vector
    std::vector<std::pair<input_vector, double>> sample_vector;
    for (std::pair<double, double> sample : samples)
    {
        input_vector input;
        input(0) = sample.first;

        std::pair<input_vector, double> newPair(input, sample.second);

        sample_vector.push_back(newPair);
    }

    double bestError = std::numeric_limits<double>::infinity();
    parameter_vector best_parameters;
    for (int i=0; i<nIterations; i++)
    {
        parameter_vector parameters = getRandomParameterVector(samples);
//        qDebug() << i << ")";
//        qDebug() << parameters(0) << ", " << parameters(1) << ", " << parameters(2) << ", " <<  parameters(3) << parameters(4) << parameters(5) << parameters(6) << parameters(7);

        // start solver
        dlib::solve_least_squares(
                    dlib::objective_delta_stop_strategy(1e-7, LEAST_SQUARES_MAX_ITERATIONS),
                    [this](const std::pair<input_vector, double>& data, const parameter_vector& params) -> double
                      { return residual(data, params);},
                    [this](const std::pair<input_vector, double>& data, const parameter_vector& params) -> parameter_vector
                      { return  residual_derivative(data, params);},
                    sample_vector,
                    parameters
        );

//        qDebug() << parameters(0) << ", " << parameters(1) << ", " << parameters(2) << ", " <<  parameters(3) << parameters(4) << parameters(5) << parameters(6) << parameters(7);

        double error = residual_sum_of_sqares(samples, parameters);
//        qDebug() << "Error:\t" << QString::number(error);

        // check f(t->infinty)
        // if f(t->infinity) > 2 * y_max:
        // ignore result
        input_vector testInput = {100000.};
        double testOutput = model(testInput, parameters);
        if (testOutput > limitFactor * y_max)
        {
//          qDebug() << "Plateau = " << QString::number(parameters(0) + parameters(3)) << "\t-> result ignored";
            continue;
        }

        if (error < bestError)
        {
            bestError = error;
            best_parameters = parameters;
        }
    }
    params = best_parameters;
//    qDebug() << "-> Best error:\t" << QString::number(bestError);
}

void LeastSquaresFitter::solve_lm(const std::vector<std::pair<double, double> > &samples, int nIterations, double limitFactor)
{
    qDebug() << "solve_lm";
    // find y_max
    double y_max = 0.;
    for (auto pair : samples)
    {
        if (pair.second > y_max)
            y_max = pair.second;
    }

    // prepare sample_vector
    std::vector<std::pair<input_vector, double>> sample_vector;
    for (std::pair<double, double> sample : samples)
    {
        input_vector input;
        input(0) = sample.first;

        std::pair<input_vector, double> newPair(input, sample.second);

        sample_vector.push_back(newPair);
    }

    double bestError = std::numeric_limits<double>::infinity();
    parameter_vector best_parameters;
    for (int i=0; i<nIterations; i++)
    {
        parameter_vector parameters = getRandomParameterVector(samples);

        // start solver
        dlib::solve_least_squares_lm(
                    dlib::objective_delta_stop_strategy(1e-7, LEAST_SQUARES_MAX_ITERATIONS),
                    [this](const std::pair<input_vector, double>& data, const parameter_vector& params) -> double
                      { return residual(data, params);},
                    [this](const std::pair<input_vector, double>& data, const parameter_vector& params) -> parameter_vector
                      { return  residual_derivative(data, params);},
                    sample_vector,
                    parameters
        );

        double error = residual_sum_of_sqares(samples, parameters);
//        qDebug() << "Error:\t" << QString::number(error);

        // check f(t->infinty)
        // if f(t->infinity) > 2 * y_max:
        // ignore result
        input_vector testInput = {100000.};
        double testOutput = model(testInput, parameters);
        if (testOutput > limitFactor * y_max)
        {
//            qDebug() << "Plateau = " << QString::number(parameters(0) + parameters(3)) << "\t-> result ignored";
            continue;
        }

        if (error < bestError)
        {
            bestError = error;
            best_parameters = parameters;
        }
    }
    params = best_parameters;
//    qDebug() << "-> Best error:\t" << QString::number(bestError);
}

double LeastSquaresFitter::residual_sum_of_sqares(const std::vector<std::pair<double, double> > &samples) const
{
    return residual_sum_of_sqares(samples, params);
}

double LeastSquaresFitter::residual_sum_of_sqares(const std::vector<std::pair<double, double> > &samples, const parameter_vector &parameters) const
{
    double sum = 0.;
    for (std::pair<double, double> sample : samples)
    {
        // convert to input_vector
        input_vector input;
        input(0) = sample.first;

        std::pair<input_vector, double> pair(input, sample.second);

        // calculate residual
        double res = residual(pair, parameters);
        sum += res*res;
    }

    return sum;
}

std::vector<double> LeastSquaresFitter::getParams() const
{
    std::vector<double> param_vector;

    for (int i=0; i < params.size(); i++)
        param_vector.push_back(params(i));

    return param_vector;
}

QList<QString> LeastSquaresFitter::getParameterNames() const
{
    return parameterNames;
}

QStringList LeastSquaresFitter::getTooltips() const
{
    QStringList tooltips;

    for (QString parameterName : parameterNames)
    {
        tooltips << "parameter " + parameterName + "\n\n" + getFunctionString();
    }

    return tooltips;
}

QMap<QString, LeastSquaresFitter::Type> LeastSquaresFitter::getTypeMap()
{
    return typeMap;
}

double LeastSquaresFitter::residual(const std::pair<input_vector, double>& data, const parameter_vector& param_vector) const
{
    return model(data.first, param_vector) - data.second;
}

//ARMT0_Fitter::ARMT0_Fitter():
//    LeastSquaresFitter()
//{
//}

//double ARMT0_Fitter::model(const input_vector &input_vector, const parameter_vector &param_vector) const
//{
//    double t = input_vector(0);

//    double alpha = param_vector(0);
//    double beta = param_vector(1);
//    double gamma = param_vector(2);
//    double t0 = param_vector(3);

//    return alpha * std::exp(-beta * (t - t0)) + gamma;
//}

//parameter_vector ARMT0_Fitter::getRandomParameterVector(const std::vector<std::pair<double, double> > &samples) const
//{
//    parameter_vector parameters = dlib::randm(params.size(), 1);

//    double t_first = samples.front().first;
//    double t_last = samples.back().first;

//    // alpha is random in range (0; -2*last_sample)
//    parameters(0) = -2 * parameters(0) * samples.back().second;

//    // beta is random in range (0; 2 * (ln(10)) / (tlast - t0))
//    // formula is based on tau90 of adg
//    parameters(1) = 2 * parameters(1) * (std::log(10)) / (t_last - t_first);

//    // gamma is random in range (0; 2*last_sample)
//    parameters(2) = 2 * parameters(2) * samples.back().second;

//    // t0 = t_first +- (0; 20)
//    parameters(3) = t_first + (parameters(3) - 0.5) * 20;

//    return parameters;
//}

//parameter_vector ARMT0_Fitter::residual_derivative(const std::pair<input_vector, double>& data, const parameter_vector& param_vector)
//{
//    // zero init parameter vector
//    parameter_vector der;
//    der = 0;

//    double t = data.first(0);

//    double alpha = param_vector(0);
//    double beta = param_vector(1);
//    double gamma = param_vector(2);
//    double t0 = param_vector(3);

//    der(0) = std::exp(beta * (t0 - t));
//    der(1) = alpha * (t0 - t) * std::exp(beta * (t0 - t));
//    der(2) = 1;
//    der(3) = alpha * beta * std::exp(-beta * (t - t0));

//    return der;
//}

///*!
// * \brief ARMT0_Fitter::tau_90
// * t_zero_intersection = t0 - log(- gamma / alpha) / beta
// * t_90%_intersection = t0 - log(- gamma / (10 * alpha)) / beta
// *
// * \return
// */
//double ARMT0_Fitter::tau_90()
//{
//    double alpha = params(0);
//    double beta = params(1);
//    double gamma = params(2);
//    double t0 = params(3);

//    double t_zero_intersection = t0 - std::log(- gamma / alpha) / beta;
//    double t90 = t0 - std::log(- gamma / (10 * alpha)) / beta;

//    return t90 - t_zero_intersection;
//}

//ADG_Fitter::ADG_Fitter():
//    LeastSquaresFitter()
//{
//}

//double ADG_Fitter::model(const input_vector &input_vector, const parameter_vector &param_vector) const
//{
//    double t = input_vector(0);

//    double alpha = param_vector(0);
//    double beta = param_vector(1);
//    double t0 = param_vector(2);

//    return alpha * (1 - std::exp(-beta * (t - t0)));
//}

//parameter_vector ADG_Fitter::getRandomParameterVector(const std::vector<std::pair<double, double> > &samples) const
//{
//    parameter_vector parameters = dlib::randm(params.size(), 1);

//    double t_first = samples.front().first;
//    double t_last = samples.back().first;

//    // alpha is random in range (0; 2*last_sample)
//    parameters(0) = 2 * parameters(0) * samples.back().second;

//    // beta is random in range (0; 2 * (ln(2) + ln(5)) / (tlast - t0))
//    // formula is based on tau90
//    parameters(1) = 2 * parameters(1) * (std::log(2) + std::log(5)) / (t_last - t_first);

//    // t0 = t_first +- (0; 20)
//    parameters(2) = t_first + (parameters(2) - 0.5) * 20;

//    return parameters;
//}

//parameter_vector ADG_Fitter::residual_derivative(const std::pair<input_vector, double>& data, const parameter_vector& param_vector)
//{
//    // zero init parameter vector
//    parameter_vector der;
//    der = 0;

//    double t = data.first(0);

//    double alpha = param_vector(0);
//    double beta = param_vector(1);
//    double t0 = param_vector(2);

//    der(0) = 1 - std::exp(-beta * (t - t0));
//    der(1) = alpha * (t - t0) * std::exp(-beta * (t - t0));
//    der(2) = -alpha * beta * std::exp(-beta * (t - t0));

//    return der;
//}

///*!
// * \brief ARM_Fitter::tau_90
// * t_zero_intersection = t0 - log(- gamma / alpha) / beta
// * t_90%_intersection = t0 - log(- gamma / (10 * alpha)) / beta
// *
// * \return
// */
//double ADG_Fitter::tau_90()
//{
//    double alpha = params(0);
//    double beta = params(1);
//    double t0 = params(2);

//    double t_zero_intersection = t0;
//    double t90 = t0 + (std::log(2) + std::log(5)) / beta;

//    return t90 - t_zero_intersection;
//}

ADG_superpos_Fitter::ADG_superpos_Fitter():
    LeastSquaresFitter()
{
    parameterNames = {QString::fromUtf8("alpha_1"),     // alpha_1
                      QString::fromUtf8("beta_1"),      // beta_1
                      QString::fromUtf8("t0_1"),        // t0_1
                      QString::fromUtf8("alpha_2"),     // alpha_2
                      QString::fromUtf8("beta_2"),      // beta_2
                      QString::fromUtf8("t0_2")         // t0_2
                     };
}

double ADG_superpos_Fitter::model(const input_vector &input_vector, const parameter_vector &param_vector) const
{
    double t = input_vector(0);

    double alpha_1 = param_vector(0);
    double beta_1 = param_vector(1);
    double t0_1 = param_vector(2);
    double alpha_2 = param_vector(3);
    double beta_2 = param_vector(4);
    double t0_2 = param_vector(5);

    return alpha_1 * (1 - std::exp(-beta_1 * (t - t0_1))) + alpha_2 * (1 - std::exp(-beta_2 * (t - t0_2)));
}

parameter_vector ADG_superpos_Fitter::getRandomParameterVector(const std::vector<std::pair<double, double> > &samples) const
{
    parameter_vector parameters = dlib::randm(params.size(), 1);

    // determine helper parameters
    double t_first = std::numeric_limits<double>::infinity();
    double t_last = 0.;
    double y_max = 0.;

    for (auto pair : samples)
    {
        if (pair.first < t_first)
            t_first = pair.first;
        if (pair.first > t_last)
            t_last = pair.first;
        if (pair.second > y_max)
            y_max = pair.second;
    }

    // determine parameters
    // alpha is random in range (0; 2*y_max)
    parameters(0) = parameters(0) * y_max;
    parameters(3) = parameters(3) * y_max;

    // beta is random in range (0; 2 * (ln(2) + ln(5)) / (tlast - t0))
    // formula is based on tau90
    parameters(1) = 2 * parameters(1) * (std::log(10)) / (t_last - t_first);
    parameters(4) = 2 * parameters(4) * (std::log(10)) / (t_last - t_first);

    // t0 = t_first +- (0; 20)
    parameters(2) = t_first + (parameters(2) - 0.5) * 20;
    parameters(5) = t_first + (parameters(5) - 0.5) * 20;

    return parameters;
}

parameter_vector ADG_superpos_Fitter::residual_derivative(const std::pair<input_vector, double>& data, const parameter_vector& param_vector)
{
    // zero init parameter vector
    parameter_vector der;
    der = 0;

    double t = data.first(0);

    double alpha_1 = param_vector(0);
    double beta_1 = param_vector(1);
    double t0_1 = param_vector(2);
    double alpha_2 = param_vector(3);
    double beta_2 = param_vector(4);
    double t0_2 = param_vector(5);

    der(0) = 1 - std::exp(-beta_1 * (t - t0_1));
    der(3) = 1 - std::exp(-beta_2 * (t - t0_2));

    der(1) = alpha_1 * (t - t0_1) * std::exp(-beta_1 * (t - t0_1));
    der(4) = alpha_1 * (t - t0_2) * std::exp(-beta_2 * (t - t0_2));

    der(2) = -alpha_1 * beta_1 * std::exp(-beta_1 * (t - t0_1));
    der(5) = -alpha_2 * beta_2 * std::exp(-beta_2 * (t - t0_2));

    return der;
}

/*!
 * \brief ARM_Fitter::tau_90
 * t_zero_intersection = t0 - log(- gamma / alpha) / beta
 * t_90%_intersection = t0 - log(- gamma / (10 * alpha)) / beta
 *
 * \return
 */
double ADG_superpos_Fitter::tau_90()
{
    double alpha_1 = params(0);
    double beta_1 = params(1);
    double t0_1 = params(2);

    double alpha_2 = params(3);
    double beta_2 = params(4);
    double t0_2 = params(5);

    double t_zero = (t0_1 + t0_2) / 2;

    dlib::find_min_single_variable(
        [this](const double input) -> double
        {
            input_vector input_vector;
            input_vector(0) = input;
            return std::abs(model(input_vector, params));
        },
        t_zero
    );

    double t_90 = (t0_1 + t0_2) / 2;

    dlib::find_min_single_variable(
        [this, alpha_1, alpha_2](const double input) -> double
        {
            input_vector input_vector;
            input_vector(0) = input;
            return std::abs(model(input_vector, params) - 0.9 * (alpha_1 + alpha_2));
        },
        t_90
    );

    return t_90 - t_zero;
}

double ADG_superpos_Fitter::f_t_90()
{
    double alpha_1 = params(0);
    double beta_1 = params(1);
    double t0_1 = params(2);

    double alpha_2 = params(3);
    double beta_2 = params(4);
    double t0_2 = params(5);

    return 0.9 * (alpha_1 + alpha_2);
}

QString ADG_superpos_Fitter::getFunctionString() const
{
    return "f(t) = alpha_1 * e^(-beta_1 * (t - t0_1)) + alpha_2 * e^(-beta_2 * (t - t0_2))";
}

//Exposition_Fitter::Exposition_Fitter():
//    LeastSquaresFitter()
//{
//}

//double Exposition_Fitter::model(const input_vector &input_vector, const parameter_vector &param_vector) const
//{
//    double t = input_vector(0);

//    double alpha_1 = param_vector(0);
//    double beta_1 = param_vector(1);
//    double alpha_2 = param_vector(2);
//    double beta_2 = param_vector(3);
//    double gamma = param_vector(4);

//    return alpha_1 * std::exp(-beta_1 * t) + alpha_2 * std::exp(-beta_2 * t) + gamma;
//}

//parameter_vector Exposition_Fitter::getRandomParameterVector(const std::vector<std::pair<double, double> > &samples) const
//{
//    parameter_vector parameters = dlib::randm(params.size(), 1);

//    double t_first = samples.front().first;
//    double t_last = samples.back().first;

//    // alpha is random in range (0; 2*last_sample)
//    parameters(0) = - 4 * parameters(0) * samples.back().second;
//    parameters(2) = - 4 * parameters(2) * samples.back().second;

//    // beta is random in range (0; 2 * (ln(10)) / (tlast - t0))
//    // formula is based on tau90
//    parameters(1) = 2 * parameters(1) * (std::log(10)) / (t_last - t_first);
//    parameters(3) = 2 * parameters(3) * (std::log(10)) / (t_last - t_first);

//    // t0 = t_first +- (0; 20)
//    parameters(4) = 2 * parameters(4) * samples.back().second;

//    return parameters;
//}

//parameter_vector Exposition_Fitter::residual_derivative(const std::pair<input_vector, double>& data, const parameter_vector& param_vector)
//{
//    // zero init parameter vector
//    parameter_vector der;
//    der = 0;

//    double t = data.first(0);

//    double alpha_1 = param_vector(0);
//    double beta_1 = param_vector(1);
//    double alpha_2 = param_vector(2);
//    double beta_2 = param_vector(3);
//    double gamma = param_vector(4);

//    der(0) = std::exp(-beta_1 * t);
//    der(2) = std::exp(-beta_2 * t);

//    der(1) = -t * alpha_1 * std::exp(-beta_1 * t);
//    der(3) = -t * alpha_2 * std::exp(-beta_2 * t);

//    der(4) = 1;

//    return der;
//}

///*!
// * \brief ARM_Fitter::tau_90
// * find t_zero: min(abs(f(t))) @ t = t_zero
// * find t_90: min(abs(f(t)-0.9*gamma)) @ t = t_90
// * tau_90 = t_90 - t_zero
// * \return
// */
//double Exposition_Fitter::tau_90()
//{
//    double alpha_1 = params(0);
//    double beta_1 = params(1);
//    double alpha_2 = params(2);
//    double beta_2 = params(3);
//    double gamma = params(4);

//    // estimate t_zero
//    double t0_1 = std::log(-alpha_1 / gamma) / beta_1;
//    double t0_2 = std::log(-alpha_2 / gamma) / beta_2;

//    double t_zero = (t0_1 + t0_2) / 2;

//    dlib::find_min_single_variable(
//        [this](const double input) -> double
//        {
//            input_vector input_vector;
//            input_vector(0) = input;
//            return std::abs(model(input_vector, params));
//        },
//        t_zero
//    );

//    // estimate t_90
//    double t90_1 = std::log(- 10 * alpha_1 / gamma) / beta_1;
//    double t90_2 = std::log(- 10 * alpha_2 / gamma) / beta_2;

//    double t_90 = (t90_1 + t90_2) / 2;

//    dlib::find_min_single_variable(
//        [this, alpha_1, alpha_2](const double input) -> double
//        {
//            input_vector input_vector;
//            input_vector(0) = input;
//            return std::abs(model(input_vector, params) - 0.9 * (alpha_1 + alpha_2));
//        },
//        t_90
//    );

//    return t_90 - t_zero;
//}

LinearFitter::LinearFitter()
{

}

double LinearFitter::model(const double input) const
{
    return m * input + b;
}

void LinearFitter::fit(const std::vector<double> &x, const std::vector<double> &y)
{
    Q_ASSERT(x.size() == y.size());

    size_t n = x.size();

    double x_mean = std::accumulate(x.begin(), x.end(), 0.0) / n;
    double y_mean = std::accumulate(y.begin(), y.end(), 0.0) / n;

    double squared_x_sum = 0.;
    double xy_sum = 0.;

    for (size_t i=0; i<n; i++)
    {
        squared_x_sum += x[i] * x[i];
        xy_sum += x[i] * y[i];
    }

    double ss_xx = squared_x_sum - n * x_mean * x_mean;
    double ss_xy = xy_sum - n * x_mean * y_mean;

    // calculate parameters
    m =  ss_xy / ss_xx;
    b = y_mean - m * x_mean;

    // calculate std deviation
    double variance = 0.;
    for (size_t i=0; i<n; i++)
        variance += std::pow(model(x[i]) - y[i], 2) / n;
    stdDev = sqrt(variance);
}

double LinearFitter::getM() const
{
    return m;
}

double LinearFitter::getB() const
{
    return b;
}

double LinearFitter::getStdDev() const
{
    return stdDev;
}
