// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "XMath.h"
#include "AxesPool.h"
#include "Components/ActorComponent.h"
#include "CameraSmartStick.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class FLYINGTEST_API UCameraSmartStick : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCameraSmartStick();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
		Sets provided scene components in the following hierarchy (parent -> child): CameraTarget -> RestingPoint -> Camera.
		Typically CameraTarget should be just a scene component attached to the actual target due to the fact
		that the Camera rotation around the target is achived by corresponding rotation of the CameraTarget 
		(in this sense CameraTarget can be thought as pivot point of an imaginary stick held the Camera).
		RestingPoint - scene component designating Camera relative location when CameraTarget velocity vector is constant
		(stick tip the Camera is attached to by damped spring)
	*/
	UFUNCTION(BlueprintCallable, Category = "Construction")
	void SetComponents(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint);


	//--------------------------------------------------------------------------------------------------------------------------
	// --- Smoothing options ---------------------------------------------------------------------------------------------------
	FORCEINLINE float GetAccelShiftMax() { return accelShiftMax; }
	FORCEINLINE void SetAccelShiftMax(float value) { accelShiftMax = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetZenithLimit() { return zenithLimit; }
	FORCEINLINE void SetZenithLimit(float value) { zenithLimit = value < 0.0f ? 0.01f : value; }

	FORCEINLINE float GetStiffness() { return stiffness; }
	FORCEINLINE void SetStiffness(float value) { stiffness = value <= 0.0f ? 0.000001f : value; }

	FORCEINLINE float GetDamping() { return damping; }
	FORCEINLINE void SetDamping(float value) { damping = value <= 0.0f ? 0.000001f : value; }

	FORCEINLINE float GetAccelAlpha() { return accelAlpha; }
	FORCEINLINE void SetAccelAlpha(float value) { accelAlpha = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value); }


	// -------------------------------------------------------------------------------------------------------------------------------
	// --- Obstacle Detection --------------------------------------------------------------------------------------------------------
	/*
		If true enables periodic trace from the CameraTarget to the Camera to avoid obstacles
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection")
	bool traceEnabled;

	FORCEINLINE float GetTraceRadius() { return traceRadius; }
	FORCEINLINE void SetTraceRadius(float value) { traceRadius = value < 0.0f ? 0.0f : value; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection")
	TEnumAsByte<ETraceTypeQuery> traceChannel;

	FORCEINLINE float GetTraceRate() { return traceRate; }
	FORCEINLINE void SetTraceRate(float value) { traceRate = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetTraceAlpha() { return traceAlpha; }
	FORCEINLINE void SetTraceAlpha(float value) { traceAlpha = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value); }

	FORCEINLINE float GetTraceInterval() { return traceInterval; }
	FORCEINLINE void SetTraceInterval(float value) { traceInterval = value < 0.0f ? 0.0f : value; }

	/*
		The array of actors that should be ignored by the trace.
		The actor owning this component is added to the array in the SetComponent method
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection")
	TArray<AActor*> traceIgnore;


	// ------------------------------------------------------------------------------------------------------------
	// --- Axes Input ---------------------------------------------------------------------------------------------
	/*
		Sets position of the RestingPoint and orientation of the Camera accordingly to the provided axes values.
		If RestingPoint is currently being transferred to one of the preset position
		then only camera rotation-input will have effect
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input")
	void AxesPoolInput(FAxesPool input);


	// ---------------------------------------------------------------------------------------------
	// --- Axes Input | Stick Rotation -------------------------------------------------------------
	/*
		If true then RestingPoint new location remains after axes input values become zero,
		otherwise, RestingPoint returns to initial location when axes input values become zero
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation")
	bool stickRotPermanent;

	FORCEINLINE float GetStickRotRate() { return stickRotRate; }
	FORCEINLINE void SetStickRotRate(float value) { stickRotRate = value < 0.0f ? 0.0f : (value > 180.0f ? 180.0f : value); }

	FORCEINLINE float GetStickRotAlpha() { return stickRotAlpha; }
	FORCEINLINE void SetStickRotAlpha(float value) { stickRotAlpha = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value); }

	/*
		Rotates imaginary stick causing changing of the RestingPoint relative location.
		The yaw and pitch are axes input values. The axis scale of 1.0 is expected for each input.
		Does not check if RestingPoint is being transferred now to one of the preset position
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Stick Rotation")
	void RotateStick(float yaw, float pitch, float deltaTime);


	// --------------------------------------------------------------------------------------------
	// --- Axes Input | Camera Rotation -----------------------------------------------------------
	/*
		The Camera angular offset from the look at the CameraTarget direction
	*/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Axes Input|Camera Rotation")
	FRotator cameraRotOffset;

	/*
		If true then new offset from the look at the CameraTarget remains after axes input values become zero,
		otherwise, Camera will look at the CameraTarget when axes input values become zero
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation")
	bool cameraRotPermanent;

	FORCEINLINE float GetCameraRotRate() { return cameraRotRate; }
	FORCEINLINE void SetCameraRotRate(float value) { cameraRotRate = value < 0.0f ? 0.0f : (value > 180.0f ? 180.0f : value); }

	FORCEINLINE float GetCameraRotAlpha() { return cameraRotAlpha; }
	FORCEINLINE void SetCameraRotAlpha(float value) { cameraRotAlpha = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value); }

	FORCEINLINE float GetCameraPitchLimit() { return cameraPitchLimit; }
	FORCEINLINE void SetCameraPitchLimit(float value) { cameraPitchLimit = fabsf(value); }

	FORCEINLINE float GetCameraYawLimit() { return cameraYawLimit; }
	FORCEINLINE void SetCameraYawLimit(float value) { cameraYawLimit = fabsf(value); }

	/*
		The yaw and pitch are axes input values. The axis scale of 1.0 is expected for each input
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Camera Rotation")
	void RotateCamera(float yaw, float pitch, float deltaTime);


	// -------------------------------------------------------------------------------------
	// --- Axes Input | Movement -----------------------------------------------------------
	FORCEINLINE float GetMoveRate() { return moveRate; }
	FORCEINLINE void SetMoveRate(float value) { moveRate = value < 0.0f ? 0.0f : value; }

	/*
		Corrects provided values if needed (0 <= min <= max) and
		sets the min and max distances from the CameraTarget to the RestingPoint
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Camera Rotation")
	void SetRange(float min, float max);

	/*
		Moves the RestingPoint toward the CameraTarget if input is positive and
		away from the CameraTarget if input is negative.
		Does not check if RestingPoint is being transferred now to one of the preset position
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Camera Rotation")
	void ChangeDistance(float input, float deltaTime);


	// -------------------------------------------------------------------------------------------------------
	// --- Preset Positions ----------------------------------------------------------------------------------
	/*
		Time in sec to transfer the RestingPoint to one of the preset position
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset Positions", meta = (ClampMin = "0.0"))
	float transferDuration;

	/*
		Stores current relative position of the RestingPoint into the array to the specified index.
		Position previously stored under the same index will be rewritten by the current one.
		This method is called in the BeginPlay event of this component with index equal zero 
		thereby storing initial position to the zero index
	*/
	UFUNCTION(BlueprintCallable, Category = "Preset Position")
	void StorePosition(int32 index);

	/*
		Starts transferring of the RestingPoint to the preset position with the specified index and
		returns true if the presetPositions contains record with the specified index, otherwise returns false
	*/
	UFUNCTION(BlueprintCallable, Category = "Preset Position")
	bool TransferToPreset(int32 index);

	/*
		Stores preset positions of the RestingPoint:
			FDistanceRotator.Distance is relative X-coordinate of the RestingPoint;
			FDistanceRotator.Rotation is relative rotation of the CameraTarget
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset Positions")
	TMap<int32, FDistanceRotator> presetPositions;


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// --- Pivots ----------------------------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	USceneComponent* pCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	USceneComponent* pCameraTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	USceneComponent* pRestingPoint;

	bool ValidatePivots(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint);


	//--------------------------------------------------------------------------------------------------------------------------
	// --- Smoothing options ---------------------------------------------------------------------------------------------------
	/*
		Restricts Camera shift from the RestingPoint
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.0"), BlueprintSetter=accelShiftMax_BPSet)
	float accelShiftMax;
	UFUNCTION(BlueprintSetter)
	void accelShiftMax_BPSet(float value);

	/*
		Minimal distance from the RestingPoint to the Z-axis of the CameraTarget. 
		Typically the value is set close to the AccelShiftMax to prevent the Camera to be in zenith and nadir relative to the CameraTarget. 
		Should be greater than zero
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.01"), BlueprintSetter = zenithLimit_BPSet)
	float zenithLimit;
	UFUNCTION(BlueprintSetter)
	void zenithLimit_BPSet(float value);

	/*
		Stiffness of an imaginary spring pulling the Camera to the RestingPoint.
		Less the value the more Camera shift from the RestingPoint when the CameraTarget moves with acceleration
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.000001"), BlueprintSetter=stiffness_BPSet)
	float stiffness;
	UFUNCTION(BlueprintSetter)
	void stiffness_BPSet(float value);

	/*
		Damping coefficient of an imaginary spring pulling the Camera to the RestingPoint.
		Less the value the faster Camera responds to the acceleration changes of the CameraTarget.
		Too low values can cause the abrupt Camera movements. In such cases, try to reduce accelAlpha value
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.000001"), BlueprintSetter=damping_BPSet)
	float damping;
	UFUNCTION(BlueprintSetter)
	void damping_BPSet(float value);

	/*
		Smoothing factor of the low-pass filter applied to the Camera movement.
		Typically you start from choosing of the Stiffness and Damping with accelAlpha equal 1 and
		reduce accelAlpha value in cases of abrupt Camera movements.
		Allowed values are in range [0, 1]:
			equal to 0 - the Camera will never leave the RestingPoint;
			close to 0 - strong smoothing;
			close to 1 - slight smoothing;
			equal to 1 - no smoothing at all
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.0", ClampMax = "1.0"), BlueprintSetter=accelAlpha_BPSet)
	float accelAlpha;
	UFUNCTION(BlueprintSetter)
	void accelAlpha_BPSet(float value);


	// -------------------------------------------------------------------------------------------------------------------------------
	// --- Obstacle Detection --------------------------------------------------------------------------------------------------------
	/*
		Radius of the sphere used for the trace
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter=traceRadius_BPSet)
	float traceRadius;
	UFUNCTION(BlueprintSetter)
	void traceRadius_BPSet(float value);

	/*
		Defines the Camera movement speed along the trace line
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter=traceRate_BPSet)
	float traceRate;
	UFUNCTION(BlueprintSetter)
	void traceRate_BPSet(float value);

	/*
		Smoothing factor of the low-pass filter applied to the Camera movement along the trace line.
		Allowed values are in range [0, 1]:
			equal to 0 - camera will not move along trace line;
			close to 0 - strong smoothing, slow response to an obstacle;
			close to 1 - slight smoothing, fast response to an obstacle;
			equal to 1 - no smothing at all, the response depends only on the traceRate
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0", ClampMax = "1.0"), BlueprintSetter=traceAlpha_BPSet)
	float traceAlpha;
	UFUNCTION(BlueprintSetter)
	void traceAlpha_BPSet(float value);

	/*
		The time in seconds between two consecutive traces.
		If equal 0 then trace will be performed every frame.
		If jagged movement of the camera occurs with traceInterval values greater than 0 try to reduce TraceAlpha
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter=traceInterval_BPSet)
	float traceInterval;
	UFUNCTION(BlueprintSetter)
	void traceInterval_BPSet(float value);


	// ---------------------------------------------------------------------------------------------
	// --- Axes Input | Stick Rotation -------------------------------------------------------------
	/*
		In degrees per sec
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0"), BlueprintSetter=stickRotRate_BPSet)
	float stickRotRate;
	UFUNCTION(BlueprintSetter)
	void stickRotRate_BPSet(float value);

	/*
		Smoothing factor of the low-pass filter applied to the axes input.
		Filtering is applied when stickRotPermanent is false only.
		Allowed values are in range [0, 1]:
			equal to 0 - result axes input will always be zero;
			close to 0 - strong smoothing;
			close to 1 - slight smoothing;
			equal to 1 - no smothing at all
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation", meta = (ClampMin = "0.0", ClampMax = "1.0"), BlueprintSetter=stickRotAlpha_BPSet)
	float stickRotAlpha;
	UFUNCTION(BlueprintSetter)
	void stickRotAlpha_BPSet(float value);


	// --------------------------------------------------------------------------------------------
	// --- Axes Input | Camera Rotation -----------------------------------------------------------
	/*
		In degrees per sec
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0"), BlueprintSetter=cameraRotRate_BPSet)
	float cameraRotRate;
	UFUNCTION(BlueprintSetter)
	void cameraRotRate_BPSet(float value);

	/*
		Smoothing factor of the low-pass filter applied to the axes input.
		Filtering is applied when headRotationPermanent is false only.
		Allowed values are in range [0, 1]:
			equal to 0 - result axes input will always be zero;
			close to 0 - strong smoothing;
			close to 1 - slight smoothing;
			equal to 1 - no smothing at all
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0", ClampMax = "1.0"), BlueprintSetter=cameraRotAlpha_BPSet)
	float cameraRotAlpha;
	UFUNCTION(BlueprintSetter)
	void cameraRotAlpha_BPSet(float value);

	/*
		Absolute limit in degrees. Values greater than 180.0 set unlimited pitch change
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0"))
	float cameraPitchLimit;

	/*
		Absolute limit in degrees. Values greater than 180.0 set unlimited yaw change
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0"))
	float cameraYawLimit;


	// -------------------------------------------------------------------------------------
	// --- Axes Input | Movement -----------------------------------------------------------
	/*
		In cm per sec
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Movement", meta = (ClampMin = "0.0"), BlueprintSetter=moveRate_BPSet)
	float moveRate;
	UFUNCTION(BlueprintSetter)
	void moveRate_BPSet(float value);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Axes Input|Movement")
	float distanceMin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Axes Input|Movement")
	float distanceMax;



private:
	// --- Service variables ---------------------------------------------------------------------------------
	FHitResult svcHitResult; // tmp

	/*
		Camera movement to the rest
			svcRestPo, svcRestVo      - in world frame
			svcStepToRest, svcCamFake - in pRestingPoint frame
	*/
	FVector svcRestPo, svcRestVo, svcStepToShift, svcCamFake;

	/*
		Obstacle Detection
		All vectors in pRestingPoint frame
			svcDeltaToHit  - from pCameraTarget to hit location
			svcDeltaToFake - from svcCamFake to pCamera
	*/
	FVector svcDeltaToHit, svcDeltaToFake, svcStepToHit;
	bool svcWasHit;
	float svcHitTimer;

	/*
		Rotation around the target in degrees
	*/ 
	float stickPitchMin, stickPitchMax, stickPitchLim, stickPitch, stickYaw;
	FRotator stickIniRot;

	/*
		Preset positions
	*/
	float trfTimer, trfXa, trfXb;
	FRotator trfRotA, trfRotB, trfRotBnorm;
	bool isTransferred;


	// --- Service methods ---------------------------------------------------------------------------------
	void LookAtTarget();

	void MoveTowardShift(float dt);

	FVector HitTest(float dt, const FTransform& restTfm);

	/*
		Serves to avoid Camera locations in nadir and zenith relative to the CameraTarget 
		and also adjusts RestingPoint relative location and CameraTarget relative rotation to place
		the Camera on the negative half of X-axis of the CameraTarget.
	*/
	void CorrectRestingLocation();

	void SetRotLimits();

	void TransferRestingPoint(float deltaTime);
};
