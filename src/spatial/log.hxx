//
// Copyright (c) 2015-2020 CNRS INRIA
//

#ifndef __pinocchio_spatial_log_hxx__
#define __pinocchio_spatial_log_hxx__

namespace pinocchio
{
  /// \brief Generic evaluation of log3 function
  template<typename _Scalar>
  struct log3_impl
  {
    template<typename Matrix3Like, typename Vector3Out>
    static void run(const Eigen::MatrixBase<Matrix3Like> & R,
                    typename Matrix3Like::Scalar & theta,
                    const Eigen::MatrixBase<Vector3Out> & res)
    {
      PINOCCHIO_ASSERT_MATRIX_SPECIFIC_SIZE(Matrix3Like, R, 3, 3);
      PINOCCHIO_ASSERT_MATRIX_SPECIFIC_SIZE(Vector3Out, res, 3, 1);
      using namespace internal;

      typedef typename Matrix3Like::Scalar Scalar;
      typedef Eigen::Matrix<Scalar,3,1,PINOCCHIO_EIGEN_PLAIN_TYPE(Matrix3Like)::Options> Vector3;
    
      static const Scalar PI_value = PI<Scalar>();
    
      Scalar tr = R.trace();
      theta = if_then_else(GE,tr,Scalar(3),
                           Scalar(0), // then
                           if_then_else(LE,tr,Scalar(-1),
                                        PI_value, // then
                                        math::acos((tr - Scalar(1))/Scalar(2)) // else
                                        )
                           );
      tr = math::max(math::min(tr,Scalar(3)),Scalar(-1));

      assert(check_expression_if_real<Scalar>(theta == theta) && "theta contains some NaN"); // theta != NaN
      
      Vector3Out & res_ = PINOCCHIO_EIGEN_CONST_CAST(Vector3Out,res);
      
      // From runs of hpp-constraints/tests/logarithm.cc: 1e-6 is too small.
      const Scalar PI_value_lower = PI_value - 1e-2;
      const Scalar t = if_then_else(GT,theta,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                    theta / sin(theta), // then
                                    Scalar(1) // else
                                    ) / Scalar(2);
      const Scalar cphi = -(tr - Scalar(1))/Scalar(2);
      const Scalar beta = theta*theta / (Scalar(1) + cphi);
      const Vector3 tmp((R.diagonal().array() + cphi) * beta);
      
      res_[0] = if_then_else(GE,theta,PI_value_lower,
                               if_then_else(GT,R(2, 1),R(1, 2),Scalar(1),Scalar(-1))
                             * if_then_else(GT,tmp[0],Scalar(0),sqrt(tmp[0]),Scalar(0)), // then
                             t * (R(2, 1) - R(1, 2)) // else
                             );
      res_[1] = if_then_else(GE,theta,PI_value_lower,
                               if_then_else(GT,R(0,2),R(2,0),Scalar(1),Scalar(-1))
                             * if_then_else(GT,tmp[1],Scalar(0),sqrt(tmp[1]),Scalar(0)), // then
                             t * (R(0,2) - R(2,0)) // else
                             );
      res_[2] = if_then_else(GE,theta,PI_value_lower,
                               if_then_else(GT,R(1,0),R(0,1),Scalar(1),Scalar(-1))
                             * if_then_else(GT,tmp[2],Scalar(0),sqrt(tmp[2]),Scalar(0)), // then
                             t * (R(1,0) - R(0,1)) // else
                             );
    }
  };

  /// \brief Generic evaluation of Jlog3 function
  template<typename _Scalar>
  struct Jlog3_impl
  {
    template<typename Scalar, typename Vector3Like, typename Matrix3Like>
    static void run(const Scalar & theta,
                    const Eigen::MatrixBase<Vector3Like> & log,
                    const Eigen::MatrixBase<Matrix3Like> & Jlog)
    {
      PINOCCHIO_ASSERT_MATRIX_SPECIFIC_SIZE(Vector3Like,  log, 3, 1);
      PINOCCHIO_ASSERT_MATRIX_SPECIFIC_SIZE(Matrix3Like, Jlog, 3, 3);
       
      using namespace internal;
      Scalar ct,st; SINCOS(theta,&st,&ct);
      const Scalar st_1mct = st/(Scalar(1)-ct);
      
      const Scalar alpha = if_then_else(LT,theta,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                        Scalar(1)/Scalar(12) + theta*theta / Scalar(720), // then
                                        Scalar(1)/(theta*theta) - st_1mct/(Scalar(2)*theta) // else
                                        );
      
      const Scalar diag_value = if_then_else(LT,theta,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                             Scalar(0.5) * (2 - theta*theta / Scalar(6)), // then
                                             Scalar(0.5) * (theta * st_1mct) // else
                                             );
        
      Matrix3Like & Jlog_ = PINOCCHIO_EIGEN_CONST_CAST(Matrix3Like,Jlog);
      Jlog_.noalias() = alpha * log * log.transpose();
      Jlog_.diagonal().array() += diag_value;
        
      // Jlog += r_{\times}/2
      addSkew(Scalar(0.5) * log, Jlog_);
    }
  };

  /// \brief Generic evaluation of log6 function
  template<typename _Scalar>
  struct log6_impl
  {
    template<typename Scalar, int Options, typename MotionDerived>
    static void run(const SE3Tpl<Scalar,Options> & M,
                    MotionDense<MotionDerived> & mout)
    {
      typedef SE3Tpl<Scalar,Options> SE3;
      typedef typename SE3::Vector3 Vector3;
      
      typename SE3::ConstAngularRef R = M.rotation();
      typename SE3::ConstLinearRef p = M.translation();
      
      using namespace internal;
      
      Scalar theta;
      const Vector3 w(log3(R,theta)); // t in [0,π]
      const Scalar t2 = theta*theta;
      
      Scalar st,ct; SINCOS(theta,&st,&ct);
      const Scalar alpha = if_then_else(LT,theta,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                        Scalar(1) - t2/Scalar(12) - t2*t2/Scalar(720), // then
                                        theta*st/(Scalar(2)*(Scalar(1)-ct)) // else
                                        );
      
      const Scalar beta = if_then_else(LT,theta,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                       Scalar(1)/Scalar(12) + t2/Scalar(720), // then
                                       Scalar(1)/t2 - st/(Scalar(2)*theta*(Scalar(1)-ct)) // else
                                       );
      
      mout.linear().noalias() = alpha * p - Scalar(0.5) * w.cross(p) + (beta * w.dot(p)) * w;
      mout.angular() = w;
    }
  };

  template<typename _Scalar>
  struct Jlog6_impl
  {
    template<typename Scalar, int Options, typename Matrix6Like>
    static void run(const SE3Tpl<Scalar, Options> & M,
                    const Eigen::MatrixBase<Matrix6Like> & Jlog)
    {
      PINOCCHIO_ASSERT_MATRIX_SPECIFIC_SIZE(Matrix6Like, Jlog, 6, 6);

      typedef SE3Tpl<Scalar,Options> SE3;
      typedef typename SE3::Vector3 Vector3;
      Matrix6Like & value = PINOCCHIO_EIGEN_CONST_CAST(Matrix6Like,Jlog);

      typename SE3::ConstAngularRef R = M.rotation();
      typename SE3::ConstLinearRef p = M.translation();
      
      using namespace internal;

      Scalar t;
      Vector3 w(log3(R,t));

      // value is decomposed as following:
      // value = [ A, B;
      //           C, D ]
      typedef Eigen::Block<Matrix6Like,3,3> Block33;
      Block33 A = value.template topLeftCorner<3,3>();
      Block33 B = value.template topRightCorner<3,3>();
      Block33 C = value.template bottomLeftCorner<3,3>();
      Block33 D = value.template bottomRightCorner<3,3>();

      Jlog3(t, w, A);
      D = A;

      const Scalar t2 = t*t;
      const Scalar tinv = Scalar(1)/t,
                   t2inv = tinv*tinv;
      
      Scalar st,ct; SINCOS (t, &st, &ct);
      const Scalar inv_2_2ct = Scalar(1)/(Scalar(2)*(Scalar(1)-ct));

      const Scalar beta = if_then_else(LT,t,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                       Scalar(1)/Scalar(12) + t2/Scalar(720), // then
                                       t2inv - st*tinv*inv_2_2ct // else
                                       );
      
      const Scalar beta_dot_over_theta = if_then_else(LT,t,TaylorSeriesExpansion<Scalar>::template precision<3>(),
                                                      Scalar(1)/Scalar(360), // then
                                                      -Scalar(2)*t2inv*t2inv + (Scalar(1) + st*tinv) * t2inv * inv_2_2ct // else
                                                      );

      const Scalar wTp = w.dot(p);
      const Vector3 v3_tmp((beta_dot_over_theta*wTp)*w - (t2*beta_dot_over_theta+Scalar(2)*beta)*p);
      // C can be treated as a temporary variable
      C.noalias() = v3_tmp * w.transpose();
      C.noalias() += beta * w * p.transpose();
      C.diagonal().array() += wTp * beta;
      addSkew(Scalar(.5)*p,C);

      B.noalias() = C * A;
      C.setZero();
    }
  };

} // namespace pinocchio

#endif // ifndef __pinocchio_spatial_log_hxx__
