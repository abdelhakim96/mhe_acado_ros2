
#include "mhe_acado.h"

using namespace Eigen;

NMHEworkspace nmheWorkspace;
NMHEvariables nmheVariables;

NMHE_FXFYFZ::NMHE_FXFYFZ(struct nmhe_struct_ &_nmhe_inp_struct)
{


    std::cout<<"Entered NMHE_FXFYFZ\n";

    is_estimator_init = false;
    is_prediction_init = false;

    nmhe_inp_struct = _nmhe_inp_struct;

    MatrixXd WL_mat_dummy(NMHE_NX,NMHE_NX);

     std::cout<<NMHE_NX <<"   NMHE_NX \n";
 /*
    for(int i = 0; i < NMHE_NX; ++i )
    {
        for(int j = 0; j < NMHE_NX; ++j )
        {
            if(i==j)
                WL_mat_dummy((i * NMHE_NX) + j) = 1/nmhe_inp_struct.process_noise_cov(i);        // Check for lower triangle Cholesky decomposition!
            else
                WL_mat_dummy((i * NMHE_NX) + j) = 0.0001;
        }
    }

*/

    Eigen::LLT<MatrixXd> lltofWL_mat_dummy(WL_mat_dummy);
    WL_mat = lltofWL_mat_dummy.matrixL();

    run_cnt = 1;

    // --------------------
    // ACADO NMHE
    // --------------------
\
    nmhe_struct.initializeSolver = boost::bind(nmhe_initializeSolver);
    nmhe_struct.preparationStep = boost::bind(nmhe_preparationStep);
    nmhe_struct.feedbackStep = boost::bind(nmhe_feedbackStep);
    nmhe_struct.getKKT = boost::bind(nmhe_getKKT);
    nmhe_struct.getObjective = boost::bind(nmhe_getObjective);
    nmhe_struct.printDifferentialVariables = boost::bind(nmhe_printDifferentialVariables);
    nmhe_struct.printControlVariables = boost::bind(nmhe_printControlVariables);
    nmhe_struct.updateArrivalCost = boost::bind(nmhe_updateArrivalCost, 0);


    nmhe_struct.acado_N = NMHE_N;
    nmhe_struct.acado_NX = NMHE_NX;
    nmhe_struct.acado_NY = NMHE_NY;
    nmhe_struct.acado_NYN = NMHE_NYN;
    nmhe_struct.acado_NU = NMHE_NU;
    nmhe_struct.acado_NOD = NMHE_NOD;

    nmhe_struct.x = &nmheVariables.x[0];
    nmhe_struct.u = &nmheVariables.u[0];
    nmhe_struct.od = &nmheVariables.od[0];
    nmhe_struct.y = &nmheVariables.y[0];
    nmhe_struct.yN = &nmheVariables.yN[0];
    nmhe_struct.W = &nmheVariables.W[0];
    nmhe_struct.WN = &nmheVariables.WN[0];
    nmhe_struct.SAC = &nmheVariables.SAC[0];
    nmhe_struct.xAC = &nmheVariables.xAC[0];
    nmhe_struct.WL = &nmheVariables.WL[0];





    nmhe_est_struct.u_est = 0.0;
    nmhe_est_struct.v_est = 0.0;
    nmhe_est_struct.w_est = 0.0;


    nmhe_est_struct.Fx_dist_est = 0.0;
    nmhe_est_struct.Fy_dist_est = 0.0;
    nmhe_est_struct.Fz_dist_est = 0.0;
    nmhe_est_struct.exe_time = 0.0;
    nmhe_est_struct.kkt_tol = 0.0;
    nmhe_est_struct.obj_val = 0.0;

   
        std::cout<<"***********************************************\n";
        std::cout<<"Constructor of the class NMHE_FXFYFZ is created\n";
        std::cout<<"***********************************************\n";

}

NMHE_FXFYFZ::~NMHE_FXFYFZ()
{
    if (nmhe_inp_struct.verbose)
        std::cout<<"Destructor of the class NMHE_FXFYFZ\n";
}

bool NMHE_FXFYFZ::return_estimator_init_value()
{
    return NMHE_FXFYFZ::is_estimator_init;
}

void NMHE_FXFYFZ::nmhe_init(struct acado_struct &acadostruct)
{
    std::cout << "*********************************\n";
    std::cout << "NMHE_FXFYFZ_initEstimator - start\n";

    // Initialize the solver
    std::cout << "Initializing solver...\n";
    acadostruct.initializeSolver();
    
    // NMHE: initialize/set the states
    std::cout << "Initializing states...\n";
    for (int i = 0; i < acadostruct.acado_N + 1; ++i)
    {
        for (int j = 0; j < acadostruct.acado_NX; ++j)
        {

                            std::cout <<acadostruct.x[0] << "acadostruct.x[0]\n";
                            std::cout <<nmhe_inp_struct.X0[0] << "nmhe_inp_struct.X0[0]\n";

                std::cout <<j << "j\n";
                std::cout <<i << "i\n";

            acadostruct.x[(i * acadostruct.acado_NX) + j] = nmhe_inp_struct.X0[j];
        }
    }
    
    // NMHE: initialize/set the controls
    std::cout << "Initializing controls...\n";
    for (int i = 0; i < acadostruct.acado_NU * acadostruct.acado_N; ++i)
    {
        acadostruct.u[i] = 0.0;
    }

    // NMHE: initialize/set the online data
    std::cout << "Initializing online data...\n";
    for (int i = 0; i < acadostruct.acado_NOD * (acadostruct.acado_N + 1); ++i)
    {
        acadostruct.od[i] = 0.0;
    }

    // NMHE: initialize the measurements/reference
    std::cout << "Initializing measurements/reference...\n";
    for (int i = 0; i < acadostruct.acado_NY * acadostruct.acado_N; ++i)
    {
        acadostruct.y[i] = 0.0;
    }
    for (int i = 0; i < acadostruct.acado_NYN; ++i)
    {
        acadostruct.yN[i] = 0.0;
    }

    // NMHE: initialize the weight matrices
    std::cout << "Initializing weight matrices...\n";
    for (int i = 0; i < acadostruct.acado_NY; ++i)
    {
        for (int j = 0; j < acadostruct.acado_NY; ++j)
        {
            acadostruct.W[(i * acadostruct.acado_NY) + j] = (i == j) ? nmhe_inp_struct.W[i] : 0.0;
        }
    }
    for (int i = 0; i < acadostruct.acado_NYN; ++i)
    {
        for (int j = 0; j < acadostruct.acado_NYN; ++j)
        {
            acadostruct.WN[(i * acadostruct.acado_NYN) + j] = (i == j) ? nmhe_inp_struct.WN[i] : 0.0;
        }
    }

    // NMHE: initialize the arrival cost
    std::cout << "Initializing arrival cost...\n";
    for (int i = 0; i < acadostruct.acado_NX; ++i)
    {
        for (int j = 0; j < acadostruct.acado_NX; ++j)
        {
            acadostruct.SAC[(i * acadostruct.acado_NX) + j] = (i == j) ? 1 / nmhe_inp_struct.SAC[i] : 0.0001;
        }
    }
    for (int i = 0; i < acadostruct.acado_NX; ++i)
    {
        for (int j = 0; j < acadostruct.acado_NX; ++j)
        {
            acadostruct.WL[(i * acadostruct.acado_NX) + j] = (i >= j) ? WL_mat(i) : 0.0001;
        }
    }

    // Prepare first step
    std::cout << "Preparing first step...\n";
    acadostruct.preparationStep();

    std::cout << "Updating arrival cost matrix...\n";
    acadostruct.updateArrivalCost(1); // pass 1 to init the SAC matrix

    // Check for NaN values in SAC
    std::cout << "Checking SAC for NaN values...\n";
    MatrixXd SAC_dummy(acadostruct.acado_NX, acadostruct.acado_NX);
    int is_nan = 0;
    for (int i = 0; i < acadostruct.acado_NX; ++i)
    {
        for (int j = 0; j < acadostruct.acado_NX; ++j)
        {
            SAC_dummy(i, j) = nmheVariables.SAC[(i * acadostruct.acado_NX) + j];
            if (std::isnan(SAC_dummy(i, j)))
            {
                is_nan++;
                std::cout << "NaN detected at SAC[" << i << "][" << j << "]\n";
            }
        }
    }
  //  if (is_nan != 0)
   // {
   //     std::cerr << "nmheVariables.SAC has " << is_nan << " NaNs! Reinitializing SAC matrix!\n";
   //     exit(1); // Exiting the program if NaNs are found
   // }

    //if (nmhe_inp_struct.verbose)
    //{
        std::cout << "NMHE_FXFYFZ: initialized correctly\n";
        std::cout << "**********************************\n";
    //}

    is_estimator_init = true;
}


void NMHE_FXFYFZ::nmhe_core(struct acado_struct &acadostruct, struct estimation_struct &estimationstruct,
                            Eigen::VectorXd &currentvelrate, Eigen::VectorXd &control_cmd)
{


    Vector4d nmpc_cmd;
    nmpc_cmd << control_cmd(0), control_cmd(1), control_cmd(2), control_cmd(3);




    // set the measurement feedback
    set_measurements(acadostruct, currentvelrate, nmpc_cmd);

    // NMHE: calc and give estimation and prepare optimization for the next step
    // ----------------------------------------------------------------------

    // Execute Calculation (Optimization)
    clock_t stopwatch;
    stopwatch = clock();
    acado_feedbackStep_fb = acadostruct.feedbackStep();

    if (nmhe_inp_struct.verbose && acado_feedbackStep_fb != 0)
    {
        std::cout<<"ACADO ERROR: " << acado_feedbackStep_fb<<"\n";
        std::cout<<
            "acado NMHE_mLearning states: u, v, w, Fx_dist, Fy_dist, Fz_dist  = "
            << acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX)] << ", "
            << acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 1] << ", "
            << acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 2] << ", "
            << acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 3] << ", "
            << acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 4] << ", "
            << acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 5] << "\n";
    }

    // Feedback the new estimation immediately to the process, first NU components.

    if (is_prediction_init)
    {
        estimationstruct.u_est = acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX)];
        estimationstruct.v_est = acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 1];
        estimationstruct.w_est = acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 2];
        estimationstruct.Fx_dist_est = acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 3];
        estimationstruct.Fy_dist_est = acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 4];
        estimationstruct.Fz_dist_est = acadostruct.x[(acadostruct.acado_N*acadostruct.acado_NX) + 5];
        estimationstruct.exe_time = ((double)(clock() - stopwatch))/CLOCKS_PER_SEC;
        estimationstruct.kkt_tol = acadostruct.getKKT();
        estimationstruct.obj_val = acadostruct.getObjective();
    }

    acadostruct.updateArrivalCost(0);                       // pass 0 to just update the arrival cost



    // Settings for the next iteration
    acadostruct.preparationStep();


}








void NMHE_FXFYFZ::set_measurements(struct acado_struct &acadostruct, VectorXd &currentvelrates,
                                   Vector4d &nmpccmd)
{
    // Fill in the measurement buffer, entries 1: N
    if (run_cnt < (acadostruct.acado_N + 1))
    {
        for(int i = 0; i < acadostruct.acado_NYN; ++i )
        {
            acadostruct.y[(run_cnt-1)*acadostruct.acado_NY + i] = currentvelrates(i);
        }

        if (run_cnt > 1)
        {
            for(int i = acadostruct.acado_NYN; i < acadostruct.acado_NY; ++i )
            {
                acadostruct.y[(run_cnt-2)*acadostruct.acado_NY + i] = nmpccmd(i - acadostruct.acado_NYN);
            }
        }

        // Initialize solver, measured states, measured control and online data on shooting nodes 1: N
        for(int i = 0; i < acadostruct.acado_NX-3; ++i )
        {
            acadostruct.x[(run_cnt-1)*acadostruct.acado_NX + i] = currentvelrates(i);
        }
        for(int i = 0; i < acadostruct.acado_NU; ++i )
        {
            if (run_cnt > 1)
                acadostruct.u[(run_cnt-2)*acadostruct.acado_NU + i] = nmpccmd(i);
        }
        for(int i = 0; i < acadostruct.acado_NOD; ++i )
        {
            acadostruct.od[(run_cnt-1)*acadostruct.acado_NOD + i] = currentvelrates(i + 3);
        }

        if (nmhe_inp_struct.verbose)
            std::cout<<"run_cnt = "<<run_cnt<<"\n";

        // Increment counter
        run_cnt++;
    }

    // Initialize measurements on node N + 1,
    else if(run_cnt == (acadostruct.acado_N + 1))
    {
        for(int i = 0; i < acadostruct.acado_NYN; ++i )
            acadostruct.yN[i] = currentvelrates(i);

        // Initialize measured controls on previous node, node N
        for(int i = acadostruct.acado_NYN; i < acadostruct.acado_NY; ++i )
        {
            acadostruct.y[(acadostruct.acado_N-1)*acadostruct.acado_NY + i] = nmpccmd(i - acadostruct.acado_NYN);
        }

        // Initialize measured states, measured control and online data on node N + 1
        for(int i = 0; i < acadostruct.acado_NYN; ++i )
        {
            acadostruct.x[(run_cnt-1)*acadostruct.acado_NX + i] = currentvelrates(i);
        }
        for(int i = 0; i < acadostruct.acado_NU; ++i )
        {
            acadostruct.u[(acadostruct.acado_N-1)*acadostruct.acado_NU + i] = nmpccmd(i);
        }
        for(int i = 0; i < acadostruct.acado_NOD; ++i )
        {
            acadostruct.od[(run_cnt-1)*acadostruct.acado_NOD + i] = currentvelrates(i + 3);
        }

        if (nmhe_inp_struct.verbose)
        {
            std::cout<<"run_cnt = "<<run_cnt<<"\n";
        }
        std::cout<<"*******************************\n";
        std::cout<<"NMHE_FXFYFZ: prediction started\n";
        std::cout<<"*******************************\n";
        is_prediction_init = true;

        // Increment counter
        run_cnt++;
    }

    // Shift measurements
    else
    {
        for(int i = 0; i < acadostruct.acado_N-1; ++i )
        {
            for(int j = 0; j < acadostruct.acado_NY; ++j )
                acadostruct.y[i*acadostruct.acado_NY + j] = acadostruct.y[(i+1)*acadostruct.acado_NY + j];
        }

        for(int i = 0; i < acadostruct.acado_NYN; ++i )
        {
            acadostruct.yN[i] = currentvelrates(i);
            acadostruct.y[(acadostruct.acado_N-1)*acadostruct.acado_NY + i] = acadostruct.yN[i];
        }
        for(int i = acadostruct.acado_NYN; i < acadostruct.acado_NY; ++i )
        {
            acadostruct.y[(acadostruct.acado_N-1)*acadostruct.acado_NY + i] = nmpccmd(i - acadostruct.acado_NYN);
        }
        for (int i = 0; i < acadostruct.acado_N + 1; ++i)
        {
            for(int j = 0; j < acadostruct.acado_NOD; ++j )
                acadostruct.od[(i * acadostruct.acado_NOD) + j] = currentvelrates(j + 3);
        }
    }
}

