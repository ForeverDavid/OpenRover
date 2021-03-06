#include <eigen3/Eigen/Dense>
#include "kalman_filter.h"

using Eigen::VectorXf;
using Eigen::MatrixXf;

#define Min(x, y) fminf(x, y)
#define Max(x, y) fmaxf(x, y)

static inline float Heaviside(float x) {
  return x < 0 ? 0 : 1;
}

static inline float DiracDelta(float x) {
  return x == 0;
}

EKF::EKF() : x_(15), P_(15, 15) {
  Reset();
}


void EKF::Reset() {
  x_ << 0.0,
        0.0,
        0.0,
        0.0,
        0.0,
        3.90000009536743,
        2.20000004768372,
        -0.250000000000000,
        1.70000004768372,
        -1.60000002384186,
        0.200000002980232,
        4.00000000000000,
        -35.0000000000000,
        125.000000000000,
        0.0;
  P_.setIdentity();
  P_.diagonal() << 1.00000011116208e-6,
    0.0100000007078052,
    4.00000000000000,
    1.00000000000000,
    0.160000011324883,
    0.0625000000000000,
    0.0625000000000000,
    0.0625000000000000,
    0.0625000000000000,
    0.250000000000000,
    0.250000000000000,
    0.250000000000000,
    10000.0000000000,
    10000.0000000000,
    1.00000000000000;
}

void EKF::Predict(float Delta_t, float u_M, float u_delta) {
  float v = x_[0];
  float delta = x_[1];
  float y_e = x_[2];
  float psi_e = x_[3];
  float kappa = x_[4];
  float ml_1 = x_[5];
  float ml_2 = x_[6];
  float ml_3 = x_[7];
  float ml_4 = x_[8];
  float srv_a = x_[9];
  float srv_b = x_[10];
  float srv_r = x_[11];

  float tmp0 = exp(ml_3);
  float tmp1 = tmp0*v;
  float tmp2 = fabsf(u_M);
  float tmp3 = tmp2*exp(ml_2);
  float tmp4 = tmp3*v;
  float tmp5 = tmp2*exp(ml_1)*Heaviside(u_M);
  float tmp6 = exp(ml_4);
  float tmp7 = v - 0.2;
  float tmp8 = Max(0, tmp7);
  float tmp9 = tmp6*(Heaviside(tmp8) + 1);
  float tmp10 = Heaviside(-Delta_t*(-tmp1 - tmp4 + tmp5 - tmp9) - v);
  float tmp11 = -Delta_t*(tmp1 + tmp4 - tmp5 + tmp9);
  float tmp12 = Heaviside(tmp11 + v);
  float tmp13 = Delta_t*tmp12;
  float tmp14 = tmp13*(tmp0 + tmp3 + tmp6*DiracDelta(tmp8)*Heaviside(tmp7));
  float tmp15 = -delta + srv_a*u_delta + srv_b;
  float tmp16 = Delta_t*srv_r;
  float tmp17 = fabsf(tmp15);
  float tmp18 = Min(tmp16, tmp17);
  float tmp19 = (((tmp15) > 0) - ((tmp15) < 0));
  float tmp20 = 2*tmp18*DiracDelta(tmp15) + pow(tmp19, 2)*Heaviside(tmp16 - tmp17);
  float tmp21 = sin(psi_e);
  float tmp22 = Delta_t*((1.0L/2.0L)*tmp10 + (1.0L/2.0L)*tmp14 - 1);
  float tmp23 = cos(psi_e);
  float tmp24 = Max(tmp11, -v);
  float tmp25 = Delta_t*((1.0L/2.0L)*tmp24 + v);
  float tmp26 = tmp23*tmp25;
  float tmp27 = pow(Delta_t, 2);
  float tmp28 = (1.0L/2.0L)*tmp12*tmp21*tmp27;
  float tmp29 = kappa*y_e;
  float tmp30 = tmp29 - 1;
  float tmp31 = 1.0/tmp30;
  float tmp32 = kappa*tmp31;
  float tmp33 = delta + tmp23*tmp32;
  float tmp34 = tmp21*tmp25;
  float tmp35 = (1.0L/2.0L)*tmp12*tmp27*tmp33;
  float tmp36 = 0.1*v + 1.0e-5;

  MatrixXf F(15, 15);
  F.setIdentity();
  F(0, 0) += -tmp10 - tmp14;
  F(0, 5) += tmp13*tmp5;
  F(0, 6) += -tmp13*tmp4;
  F(0, 7) += -tmp1*tmp13;
  F(0, 8) += -tmp13*tmp9;
  F(1, 1) += -tmp20;
  F(1, 9) += tmp20*u_delta;
  F(1, 10) += tmp20;
  F(1, 11) += Delta_t*tmp19*Heaviside(-tmp16 + tmp17);
  F(2, 0) += tmp21*tmp22;
  F(2, 3) += -tmp26;
  F(2, 5) += -tmp28*tmp5;
  F(2, 6) += tmp28*tmp4;
  F(2, 7) += tmp1*tmp28;
  F(2, 8) += tmp28*tmp9;
  F(3, 0) += tmp22*tmp33;
  F(3, 1) += -tmp25;
  F(3, 2) += pow(kappa, 2)*tmp26/pow(tmp30, 2);
  F(3, 3) += tmp32*tmp34;
  F(3, 4) += tmp26*tmp31*(tmp29*tmp31 - 1);
  F(3, 5) += -tmp35*tmp5;
  F(3, 6) += tmp35*tmp4;
  F(3, 7) += tmp1*tmp35;
  F(3, 8) += tmp35*tmp9;

  VectorXf Q(15);
  Q << 16, 4, pow(tmp36, 2), pow(tmp36, 2), pow(tmp36, 2), 0.0100000000000000, 0.000100000000000000, 0.000100000000000000, 0.0100000000000000, 0.000100000000000000, 0.000100000000000000, 0.000100000000000000, 1.00000000000000e-6, 1.00000000000000e-10, 1.00000000000000e-10;
  x_[0] += tmp24;
  x_[1] += tmp18*tmp19;
  x_[2] += -tmp34;
  x_[3] += -tmp25*tmp33;

  P_ = F * P_ * F.transpose();
  P_.diagonal() += Delta_t * Q;
}

bool EKF::UpdateCenterline(float a, float b, float c, float y_c, Eigen::MatrixXf Rk) {
  float y_e = x_[2];
  float psi_e = x_[3];
  float kappa = x_[4];
  float tmp0 = a*y_c;
  float tmp1 = b + 2*tmp0;
  float tmp2 = pow(tmp1, 2) + 1;
  float tmp3 = pow(tmp2, -1.0L/2.0L);
  float tmp4 = a*pow(y_c, 2) + b*y_c + c - tmp1*y_c;
  float tmp5 = 2*pow(tmp2, -1.5);
  float tmp6 = 1.0/tmp2;
  float tmp7 = 2*tmp6;
  float tmp8 = tmp1*tmp4;
  float tmp9 = 2*a;
  float tmp10 = pow(tmp2, -2.5);
  float tmp11 = 12.0*tmp1*tmp10;


  VectorXf yk(3);
  yk << -tmp3*tmp4 - y_e,
        -psi_e + atan(tmp1),
        a*tmp5 - kappa;

  MatrixXf Hk(3, 15);
  Hk << 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0;
  MatrixXf Mk(3, 4);
  Mk << tmp3*y_c*(tmp7*tmp8 + y_c), tmp8/pow(tmp2, 3.0L/2.0L), -tmp3, tmp3*tmp9*(tmp6*tmp8 + y_c),
        tmp7*y_c, tmp6, 0, a*tmp7,
        -tmp0*tmp11 + tmp5, -tmp10*tmp9*(3.0*b + 6.0*tmp0), 0, -pow(a, 2)*tmp11;
  Rk = Mk * Rk * Mk.transpose();

  MatrixXf S = Hk * P_ * Hk.transpose() + Rk;
  MatrixXf K = P_ * Hk.transpose() * S.inverse();

  x_.noalias() += K * yk;
  P_ = (MatrixXf::Identity(15, 15) - K*Hk) * P_;
  return true;
}

bool EKF::UpdateIMU(float g_z) {
  float v = x_[0];
  float delta = x_[1];
  float o_g = x_[14];


  VectorXf yk(1);
  yk << delta*v + g_z - o_g;

  MatrixXf Hk(1, 15);
  Hk << -delta, -v, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1;

  VectorXf Rk(1);
  Rk << 0.000100000000000000;

  MatrixXf S = Hk * P_ * Hk.transpose();
  S.diagonal() += Rk;
  MatrixXf K = P_ * Hk.transpose() * S.inverse();

  x_.noalias() += K * yk;
  P_ = (MatrixXf::Identity(15, 15) - K*Hk) * P_;
  return true;
}

bool EKF::UpdateEncoders(float dsdt, float fb_delta) {
  float v = x_[0];
  float delta = x_[1];
  float srvfb_a = x_[12];
  float srvfb_b = x_[13];


  VectorXf yk(2);
  yk << dsdt - 63.0316606304536*v,
        -delta*srvfb_a + fb_delta - srvfb_b;

  MatrixXf Hk(2, 15);
  Hk << 63.0316606304536, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, srvfb_a, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, delta, 1, 0;

  VectorXf Rk(2);
  Rk << 1, 1;

  MatrixXf S = Hk * P_ * Hk.transpose();
  S.diagonal() += Rk;
  MatrixXf K = P_ * Hk.transpose() * S.inverse();

  x_.noalias() += K * yk;
  P_ = (MatrixXf::Identity(15, 15) - K*Hk) * P_;
  return true;
}

