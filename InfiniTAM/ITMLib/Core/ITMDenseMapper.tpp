// Copyright 2014-2017 Oxford University Innovation Limited and the authors of
// InfiniTAM

#include "../../ORUtils/NVTimer.h"
#include "../Engines/Reconstruction/ITMSceneReconstructionEngineFactory.h"
#include "../Engines/Swapping/ITMSwappingEngineFactory.h"
#include "../Objects/RenderStates/ITMRenderState_VH.h"
#include "ITMDenseMapper.h"
using namespace ITMLib;

template <class TVoxel, class TIndex>
ITMDenseMapper<TVoxel, TIndex>::ITMDenseMapper(const ITMLibSettings *settings) {
    sceneRecoEngine =
            ITMSceneReconstructionEngineFactory::MakeSceneReconstructionEngine<
                    TVoxel, TIndex>(settings->deviceType);
    swappingEngine =
            settings->swappingMode != ITMLibSettings::SWAPPINGMODE_DISABLED
                    ? ITMSwappingEngineFactory::MakeSwappingEngine<TVoxel,
                                                                   TIndex>(
                              settings->deviceType)
                    : NULL;

    swappingMode = settings->swappingMode;
}

template <class TVoxel, class TIndex>
ITMDenseMapper<TVoxel, TIndex>::~ITMDenseMapper() {
    delete sceneRecoEngine;
    delete swappingEngine;
}

template <class TVoxel, class TIndex>
void ITMDenseMapper<TVoxel, TIndex>::ResetScene(
        ITMScene<TVoxel, TIndex> *scene) const {
    sceneRecoEngine->ResetScene(scene);
}

template <class TVoxel, class TIndex>
void ITMDenseMapper<TVoxel, TIndex>::ProcessFrame(
        const ITMView *view,
        const ITMTrackingState *trackingState,
        ITMScene<TVoxel, TIndex> *scene,
        ITMRenderState *renderState,
        bool resetVisibleList) {
    // ADDED BY WEI: voxel block allocation
    StopWatchLinux timer;
    timer.start();
    sceneRecoEngine->AllocateSceneFromDepth(
            scene, view, trackingState, renderState, false, resetVisibleList);
#ifndef COMPILE_WITHOUT_CUDA
    ORcudaSafeCall(cudaDeviceSynchronize());
#endif
    float alloc_time = timer.getDiffTime();
    printf("map.allocate %f\n", alloc_time);
    timer.reset();

    timer.start();
    sceneRecoEngine->IntegrateIntoScene(scene, view, trackingState,
                                        renderState);
#ifndef COMPILE_WITHOUT_CUDA
    ORcudaSafeCall(cudaDeviceSynchronize());
#endif
    float integrate_time = timer.getDiffTime();
    printf("map.integrate %f\n", integrate_time);

    timer.reset();
    timer.start();
    if (swappingEngine != NULL) {
        // swapping: CPU -> GPU
        if (swappingMode == ITMLibSettings::SWAPPINGMODE_ENABLED)
            swappingEngine->IntegrateGlobalIntoLocal(scene, renderState);

        // swapping: GPU -> CPU
        switch (swappingMode) {
            case ITMLibSettings::SWAPPINGMODE_ENABLED:
                swappingEngine->SaveToGlobalMemory(scene, renderState);
                break;
            case ITMLibSettings::SWAPPINGMODE_DELETE:
                swappingEngine->CleanLocalMemory(scene, renderState);
                break;
            case ITMLibSettings::SWAPPINGMODE_DISABLED:
                break;
        }
    }
#ifndef COMPILE_WITHOUT_CUDA
    ORcudaSafeCall(cudaDeviceSynchronize());
#endif
    float swap_time = timer.getDiffTime();
    printf("map.swap %f\n", swap_time);
    // END OF ADDITION
}

template <class TVoxel, class TIndex>
void ITMDenseMapper<TVoxel, TIndex>::UpdateVisibleList(
        const ITMView *view,
        const ITMTrackingState *trackingState,
        ITMScene<TVoxel, TIndex> *scene,
        ITMRenderState *renderState,
        bool resetVisibleList) {
    sceneRecoEngine->AllocateSceneFromDepth(
            scene, view, trackingState, renderState, true, resetVisibleList);
}
