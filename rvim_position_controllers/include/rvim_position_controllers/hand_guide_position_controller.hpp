#pragma once

#include <vector>
#include <string>

#include <hardware_interface/loaned_state_interface.hpp>
#include <hardware_interface/types/hardware_interface_type_values.hpp>
#include <controller_interface/controller_interface.hpp>

// #include <qpOASES/SQProblem.hpp>

#include <OsqpEigen/OsqpEigen.h>

#include <kdl_parser/kdl_parser.hpp>
#include <kdl/chain.hpp>
#include <kdl/chainjnttojacsolver.hpp>

#include <damped_least_squares.hpp>

// follow http://control.ros.org/ros2_controllers/doc/writing_new_controller.html
// see forward command controller for an example: https://github.com/ros-controls/ros2_controllers/blob/foxy/forward_command_controller/include/forward_command_controller/forward_command_controller.hpp
// cartesian controller: http://library.isr.ist.utl.pt/docs/roswiki/pr2_mechanism(2f)Tutorials(2f)Coding(20)a(20)realtime(20)Cartesian(20)controller(20)with(20)KDL.html
// replicate previous hand guiding mode

namespace rvim_position_controllers {

    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
    using RowMajorMatrixXd = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    constexpr char HW_IF_EXTERNAL_TORQUE[] = "external_torque";

    class HandGuidePositionController : public controller_interface::ControllerInterface {
        public:
            HandGuidePositionController() = default;

            controller_interface::return_type init(const std::string& controller_name) override;
            controller_interface::InterfaceConfiguration command_interface_configuration() const override;
            controller_interface::InterfaceConfiguration state_interface_configuration() const override;

            CallbackReturn on_configure(const rclcpp_lifecycle::State& previous_state) override;
            CallbackReturn on_activate(const rclcpp_lifecycle::State& previous_state) override;
            CallbackReturn on_deactivate(const rclcpp_lifecycle::State& previous_state) override;

            controller_interface::return_type update() override;

        protected:
            std::vector<std::string> joint_names_;
            std::string command_interface_name_;              // for parsing in yaml file (external_torque & position required)
            std::vector<std::string> state_interface_names_;  // for parsing in yaml file (external_torque & position required)

            std::vector<std::reference_wrapper<hardware_interface::LoanedStateInterface>> position_interfaces_;
            std::vector<std::reference_wrapper<hardware_interface::LoanedStateInterface>> external_torque_interfaces_;

            // KDL
            KDL::Tree tree_;
            KDL::Chain kdl_chain_;

            std::string chain_root_;
            std::string chain_tip_;

            KDL::JntArray q_;
            KDL::Jacobian J_;
            std::unique_ptr<KDL::ChainJntToJacSolver> jac_solver_;

            double th_f_=2.;
            double th_d_=1.;

            // frequency filtering, exponential smoothing
            std::vector<double> prev_update_ = std::vector<double>(7, 0.);
            double alpha_ = 0.98;

            // // quadratic problem, https://www.coin-or.org/qpOASES/doc/3.0/manual.pdf
            // qpOASES::Options options_;
            // std::unique_ptr<qpOASES::SQProblem> qp_;
            // bool qp_init_ = false;
            // RowMajorMatrixXd H_, A_;
            // Eigen::RowVectorXd g_, lb_, ub_, lba_, uba_;  // lba_ == uba_ -> ba_ (equality constraints)
            int nwsr_;
            double cputime_;  // max cpu time in seconds
            // // Eigen::RowVectorXd dq_;

            // osqp-eigen
            std::unique_ptr<OsqpEigen::Solver> qp_osqp_;
            Eigen::SparseMatrix<double> H_osqp_, A_osqp_;
            Eigen::VectorXd g_osqp_, lb_osqp_, ub_osqp_;
            Eigen::VectorXd dq_osqp_;
            Eigen::VectorXd dq_;
    };

}  // end of namespace rvim_position_controllers
