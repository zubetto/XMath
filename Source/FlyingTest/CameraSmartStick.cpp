// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#include "CameraSmartStick.h"


// Sets default values for this component's properties
UCameraSmartStick::UCameraSmartStick() :
	pCamera{ nullptr }, pCameraTarget{ nullptr }, pRestingPoint{ nullptr },

	accelShiftMax{ 50.0f }, zenithLimit{ 20.0f }, stiffness{ 10.0f }, damping{ 0.33f }, accelAlpha{ 0.02f },

	traceEnabled{ true }, traceRadius{ 20.0f }, traceChannel{ UEngineTypes::ConvertToTraceType(ECC_Visibility) },
	traceRate{ 10.0f }, traceAlpha{ 0.2f }, traceInterval{ 0.085f },

	svcRestVo{ 0.0f }, svcStepToShift{ 0.0f }, svcCamFake{ 0.0f },
	svcDeltaToHit{ 0.0f }, svcDeltaToFake{ 0.0f }, svcStepToHit{ 0.0f },

	trfRotB{ 0.0f }, transferDuration{1.0f},

	stickRotPermanent{ true }, stickRotRate{ 90.0f }, stickRotAlpha{ 0.1f },
	cameraRotPermanent{ true }, cameraRotRate{ 175.0f }, cameraRotAlpha{ 0.1f },
	cameraPitchLimit{ 360.0f }, cameraYawLimit{ 360.0f }, cameraRotOffset{ 0.0f },

	moveRate{ 300.0f }, distanceMin{ 100.0f }, distanceMax{ TNumericLimits<float>::Max() }
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	// ...
}

bool UCameraSmartStick::ValidatePivots(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint)
{
	bool isValid{ true };

	if (!IsValid(camera))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The camera is not provided or not valid"), *GetNameSafe(this));
		isValid = false;
	}
	else if (camera == cameraTarget || camera == restingPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The camera should not be the same SceneComponent as the cameraTarget or restingPoint"), *GetNameSafe(this));
		isValid = false;
	}

	if (!IsValid(cameraTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The cameraTarget is not provided or not valid"), *GetNameSafe(this));
		isValid = false;
	}
	else if (cameraTarget == camera || cameraTarget == restingPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The cameraTarget should not be the same SceneComponent as the camera or restingPoint"), *GetNameSafe(this));
		isValid = false;
	}

	if (!IsValid(restingPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The restingPoint is not provided or not valid"), *GetNameSafe(this));
		isValid = false;
	}
	else if (restingPoint == camera || restingPoint == cameraTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The restingPoint should not be the same SceneComponent as the camera or cameraTarget"), *GetNameSafe(this));
		isValid = false;
	}

	return isValid;
}

void UCameraSmartStick::SetComponents(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint)
{
	if (!ValidatePivots(camera, cameraTarget, restingPoint))
		return;

	pCamera = camera;
	pCameraTarget = cameraTarget;
	pRestingPoint = restingPoint;
	
	pCamera->AttachToComponent(pRestingPoint,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true));

	pRestingPoint->AttachToComponent(pCameraTarget,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true));

	pCamera->K2_SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f), false, svcHitResult, true);
	pCameraTarget->K2_SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f), false, svcHitResult, true);

	traceIgnore.Emplace(pCameraTarget->GetOwner());

	LookAtTarget();
}

void UCameraSmartStick::CorrectRestingLocation()
{
	FVector restP{ pRestingPoint->RelativeLocation };
	float distToZ{ restP.Size2D() };

	if (distToZ < zenithLimit)
	{
		if (distToZ > 0.0f)
		{
			float factor{ zenithLimit / distToZ };

			restP.X *= factor;
			restP.Y *= factor;
		}
		else
		{
			restP.X = zenithLimit;
			restP.Y = 0.0f;
		}
	}

	float yaw;
	float pitch;
	UKismetMathLibrary::GetYawPitchFromVector(-restP, yaw, pitch);

	pCameraTarget->K2_SetRelativeRotation(FRotator(pitch, yaw, 0.0f), false, svcHitResult, true);

	restP.X = -restP.Size();
	restP.Y = 0.0f;
	restP.Z = 0.0f;

	pRestingPoint->K2_SetRelativeLocation(restP, false, svcHitResult, true);
}

// Called when the game starts
void UCameraSmartStick::BeginPlay()
{
	Super::BeginPlay();

	if (!ValidatePivots(pCamera, pCameraTarget, pRestingPoint))
	{
		SetComponentTickEnabled(false);

		UE_LOG(LogTemp, Warning, 
			TEXT("%s: Scene components for the pivots are not properly set therefore ticking of this component will be disabled"),
			*GetFullNameSafe(this));
	}

	CorrectRestingLocation();
	SetRotLimits();
	ChangeDistance(0.0f, 0.0f);
	StorePosition(0);

	svcRestPo = pRestingPoint->GetComponentLocation();
}

// Called every frame
void UCameraSmartStick::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TransferRestingPoint(DeltaTime);
	MoveTowardShift(DeltaTime);
	LookAtTarget();
}


void UCameraSmartStick::LookAtTarget()
{
	float pitch{ pCameraTarget->RelativeRotation.Pitch };

	FRotator rotInUnpitched(pitch, 0.0f, 0.0f);
	FRotator rotToPitched(-pitch, 0.0f, 0.0f);

	FVector camInTarget{ pCamera->RelativeLocation + pRestingPoint->RelativeLocation };

	rotInUnpitched = UKismetMathLibrary::MakeRotFromX(rotInUnpitched.RotateVector(-camInTarget));
	
	rotInUnpitched.Pitch += cameraRotOffset.Pitch;
	rotInUnpitched.Yaw += cameraRotOffset.Yaw;

	rotToPitched = UKismetMathLibrary::ComposeRotators(rotInUnpitched, rotToPitched);

	pCamera->K2_SetRelativeRotation(rotToPitched, false, svcHitResult, true);
}

void UCameraSmartStick::MoveTowardShift(float dt)
{
	FTransform restTfm{ pRestingPoint->GetComponentTransform() };
	FVector restV{ (restTfm.GetTranslation() - svcRestPo) / dt };
	FVector accelShift{ ((svcRestVo - restV) / (dt * stiffness)).GetClampedToMaxSize(accelShiftMax) };

	// update Po and Vo
	svcRestPo = restTfm.GetTranslation();
	svcRestVo = restV;

	// in pRestingPoint frame
	accelShift = restTfm.GetRotation().UnrotateVector(accelShift);

	// apply low pass filter to update the svcStepToShift
	// Ynew = Yprev + alpha * (Xnew - Yprev)
	svcStepToShift += accelAlpha * ((accelShift - svcCamFake) * (dt / damping) - svcStepToShift);
	svcCamFake += svcStepToShift;

	if (traceEnabled)
	{
		pCamera->K2_SetRelativeLocation(HitTest(dt, restTfm), false, svcHitResult, true);
	}
	else
	{
		pCamera->K2_SetRelativeLocation(svcCamFake, false, svcHitResult, true);
	}
}

FVector UCameraSmartStick::HitTest(float dt, const FTransform& restTfm)
{
	// find trace vector in pRestingPoint frame;
	// the following expression implies that pRestingPoint relative rotation is always ZeroRotator
	FVector targetToFake{ svcCamFake + pRestingPoint->RelativeLocation };

	// shift vector from the pRestingPoint in pRestingPoint frame
	FVector hitShift;

	if (svcHitTimer > 0.0f)
	{
		svcHitTimer -= dt;

		if (svcWasHit)
		{
			// update svcDeltaToHit by orienting it along fresh trace vector;
			// here we approximate the new svcDeltaToHit by the projection on the trace line
			svcDeltaToHit = svcDeltaToHit.ProjectOnTo(targetToFake);

			hitShift = svcDeltaToHit - pRestingPoint->RelativeLocation;
		}
		else
			hitShift = svcCamFake;
	}
	else
	{
		svcHitTimer = traceInterval;

		svcWasHit = UKismetSystemLibrary::SphereTraceSingle(
											this,
											pCameraTarget->GetComponentLocation(),
											restTfm.TransformPositionNoScale(svcCamFake),
											traceRadius,
											traceChannel,
											false,	/*traceComplex*/
											traceIgnore,
											EDrawDebugTrace::None,
											svcHitResult,
											true /*ignoreSelf*/);

		if (svcWasHit)
		{
			hitShift = restTfm.InverseTransformPositionNoScale(svcHitResult.Location);
			svcDeltaToHit = hitShift + pRestingPoint->RelativeLocation;
		}
		else
			hitShift = svcCamFake;
	}

	// update svcDeltaToFake by orienting it along fresh trace vector;
	// here we approximate the new svcDeltaToFake by the projection on the trace line
	svcDeltaToFake = svcDeltaToFake.ProjectOnTo(targetToFake);

	// pCamera movement in pRestingPoint frame is decomposed into 
	// movement to the accelShift (presented by svcCamFake) and
	// movement along the trace line (presented by svcStepToHit);
	// camP is the pCamera position on the trace line and it is the first part of the decomposition
	FVector camP{ svcCamFake + svcDeltaToFake };

	// apply low pass filter to update the svcStepToHit
	// Ynew = Yprev + alpha * (Xnew - Yprev)
	svcStepToHit += traceAlpha * ((hitShift - camP) * (traceRate * dt) - svcStepToHit);

	// update svcDeltaToFake due to the camera movement along trace line
	svcDeltaToFake += svcStepToHit;

	return camP + svcStepToHit;
}

void UCameraSmartStick::SetRotLimits()
{
	stickIniRot = pCameraTarget->RelativeRotation;
	
	stickPitchLim = UKismetMathLibrary::DegAcos(zenithLimit / pRestingPoint->RelativeLocation.X);

	stickPitchMin = -stickPitchLim - stickIniRot.Pitch;
	stickPitchMax = stickPitchLim - stickIniRot.Pitch;
}

void UCameraSmartStick::RotateStick(float yaw, float pitch, float deltaTime)
{
	if (stickRotPermanent)
	{
		float da{ stickRotRate * deltaTime };

		stickYaw += yaw * da;
		stickYaw = UXMath::inlNormalizeClampedAngleD(stickYaw);

		stickPitch += pitch * da;
		stickPitch = UKismetMathLibrary::FClamp(stickPitch, stickPitchMin, stickPitchMax);
	}
	else
	{
		yaw *= 180.0f;
		pitch = UKismetMathLibrary::FClamp(90.0f * pitch, stickPitchMin, stickPitchMax);

		// Low-pass filtering
		stickYaw += stickRotAlpha * (yaw - stickYaw);
		stickPitch += stickRotAlpha * (pitch - stickPitch);
	}
	
	pCameraTarget->K2_SetRelativeRotation(FRotator(stickPitch + stickIniRot.Pitch, stickYaw + stickIniRot.Yaw, 0.0f), false, svcHitResult, true);
}

void UCameraSmartStick::RotateCamera(float yaw, float pitch, float deltaTime)
{
	if (cameraRotPermanent)
	{
		float da{ cameraRotRate * deltaTime };

		if (!UXMath::inlClampf(cameraRotOffset.Yaw + yaw * da, cameraYawLimit, cameraRotOffset.Yaw))
		{
			cameraRotOffset.Yaw = UXMath::inlNormalizeClampedAngleD(cameraRotOffset.Yaw);
		}

		if (!UXMath::inlClampf(cameraRotOffset.Pitch + pitch * da, cameraPitchLimit, cameraRotOffset.Pitch))
		{
			cameraRotOffset.Pitch = UXMath::inlNormalizeClampedAngleD(cameraRotOffset.Pitch);
		}
	}
	else
	{
		yaw *= 180.0f;
		pitch *= 180.0f;

		cameraRotOffset.Yaw += cameraRotAlpha * (UXMath::inlClampf(yaw, cameraYawLimit) - cameraRotOffset.Yaw);
		cameraRotOffset.Pitch += cameraRotAlpha * (UXMath::inlClampf(pitch, cameraPitchLimit) - cameraRotOffset.Pitch);
	}
}

void UCameraSmartStick::SetRange(float min, float max)
{
	distanceMin = min < 0.0f ? 0.0f : min;
	distanceMax = max < min ? min : max;
}

void UCameraSmartStick::ChangeDistance(float input, float deltaTime)
{
	input *= moveRate * deltaTime;
	input += pRestingPoint->RelativeLocation.X;
	input = UKismetMathLibrary::FClamp(fabsf(input), distanceMin, distanceMax); // now it is the new distance

	float pitch{ pCameraTarget->RelativeRotation.Pitch };
	float pcos{ UKismetMathLibrary::DegCos(pitch) };

	if ((pcos * input) > zenithLimit)
	{
		pitch = UKismetMathLibrary::DegAcos(zenithLimit / input); // the new pitch lim
	}
	else
	{
		input = zenithLimit / pcos;
		pitch = fabsf(pitch); // the new pitch lim
	}

	float delta{ pitch - stickPitchLim };

	stickPitchMin -= delta;
	stickPitchMax += delta;

	stickPitchLim = pitch;

	pRestingPoint->K2_SetRelativeLocation(FVector(-input, 0.0f, 0.0f), false, svcHitResult, true);
}

void UCameraSmartStick::AxesPoolInput(FAxesPool input)
{
	if (!isTransferred)
	{
		ChangeDistance(input.Distance, input.DeltaTime);
		RotateStick(input.StickYaw, input.StickPitch, input.DeltaTime);
	}

	RotateCamera(input.CameraYaw, input.CameraPitch, input.DeltaTime);
}

void UCameraSmartStick::StorePosition(int32 index)
{
	presetPositions.Emplace(index, FDistanceRotator{ pRestingPoint->RelativeLocation.X , pCameraTarget->RelativeRotation });
}

bool UCameraSmartStick::TransferToPreset(int32 index)
{
	if (presetPositions.Contains(index))
	{
		trfXa = pRestingPoint->RelativeLocation.X;
		trfXb = presetPositions[index].Distance;

		trfRotA = pCameraTarget->RelativeRotation;
		trfRotBnorm = presetPositions[index].Rotation;

		trfRotB.Pitch = trfRotBnorm.Pitch;
		trfRotB.Yaw = UXMath::inlNearestAngle(trfRotA.Yaw, trfRotBnorm.Yaw);

		trfTimer = 0.0f;
		isTransferred = true;

		return true;
	}
	else
		return false;
}

void UCameraSmartStick::TransferRestingPoint(float deltaTime)
{
	if (isTransferred)
	{
		trfTimer += deltaTime;

		if (trfTimer < transferDuration)
		{
			float alpha{ UKismetMathLibrary::FInterpEaseInOut(0.0f, 1.0f, trfTimer / transferDuration, 2.0f) };

			FRotator rot{ UKismetMathLibrary::RLerp(trfRotA, trfRotB, alpha, false) };
			pCameraTarget->K2_SetRelativeRotation(rot, false, svcHitResult, true);

			float X{ UKismetMathLibrary::Lerp(trfXa, trfXb, alpha) };
			pRestingPoint->K2_SetRelativeLocation(FVector(X, 0.0f, 0.0f), false, svcHitResult, true);
		}
		else
		{
			pCameraTarget->K2_SetRelativeRotation(trfRotBnorm, false, svcHitResult, true);
			pRestingPoint->K2_SetRelativeLocation(FVector(trfXb, 0.0f, 0.0f), false, svcHitResult, true);

			SetRotLimits();
			stickPitch = 0.0f;
			stickYaw = 0.0f;

			isTransferred = false;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// --- BlueprintSetters ------------------------------------------------------------------------------------------------
void UCameraSmartStick::accelShiftMax_BPSet(float value) { SetAccelShiftMax(value); }
void UCameraSmartStick::zenithLimit_BPSet(float value) { SetZenithLimit(value); }
void UCameraSmartStick::stiffness_BPSet(float value) { SetStiffness(value); }
void UCameraSmartStick::damping_BPSet(float value) { SetDamping(value); }
void UCameraSmartStick::accelAlpha_BPSet(float value) { SetAccelAlpha(value); }

void UCameraSmartStick::traceRadius_BPSet(float value) { SetTraceRadius(value); }
void UCameraSmartStick::traceRate_BPSet(float value) { SetTraceRate(value); }
void UCameraSmartStick::traceAlpha_BPSet(float value) { SetTraceAlpha(value); }
void UCameraSmartStick::traceInterval_BPSet(float value) { SetTraceInterval(value); }

void UCameraSmartStick::stickRotRate_BPSet(float value) { SetStickRotRate(value); }
void UCameraSmartStick::stickRotAlpha_BPSet(float value) { SetStickRotAlpha(value); }

void UCameraSmartStick::cameraRotRate_BPSet(float value) { SetCameraRotRate(value); }
void UCameraSmartStick::cameraRotAlpha_BPSet(float value) { SetCameraRotAlpha(value); }

void UCameraSmartStick::moveRate_BPSet(float value) { SetMoveRate(value); }