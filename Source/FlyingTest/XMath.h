// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XMath.generated.h"

/**
 * 
 */
UCLASS()
class FLYINGTEST_API UXMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/*
		Clamps the input in the range [-amplitude, amplitude]. 
		The amplitude should be greater than or equal zero
	*/
	static FORCEINLINE float inlClampf(float input, float amplitude)
	{
		if (fabsf(input) > amplitude)
		{
			input = copysignf(amplitude, input);
		}

		return input;
	}

	/*
		Clamps the input in the range [-amplitude, amplitude] and returns true if
		the input was clamped. The amplitude should be greater than or equal zero
	*/
	static FORCEINLINE bool inlClampf(float input, float amplitude, float& output)
	{
		if (fabsf(input) > amplitude)
		{
			input = copysignf(amplitude, input);
			output = input;
			return true;
		}
		else
		{
			output = input;
			return false;
		}
	}

	/*
		Clamps the input in the range [-amplitude, amplitude] and returns true if 
		the input was clamped. The amplitude should be greater than or equal zero
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static bool Clampf(float input, float amplitude, float& output);


	/*
		Clamps each element of the given vector into the corresponding range set by amps vector
		(e.g. x-element of the vector will be clamped into the range [-amps.x, amps.x]);
		All elements of the amps should be greater than or equal zero
	*/
	static FORCEINLINE void inlClamp(FVector& vector, const FVector& amps)
	{
		if (fabsf(vector.X) > amps.X)
			vector.X = copysignf(amps.X, vector.X);

		if (fabsf(vector.Y) > amps.Y)
			vector.Y = copysignf(amps.Y, vector.Y);

		if (fabsf(vector.Z) > amps.Z)
			vector.Z = copysignf(amps.Z, vector.Z);
	}

	/*
		Clamps each element of the given vector into the corresponding range set by amps vector
		(e.g. x-element of the vector will be clamped into the range [-amps.x, amps.x]);
		All elements of the amps should be greater than or equal zero
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static FVector& ClampRef(UPARAM(ref) FVector& vector, const FVector& amps);

	/*
		Returns clamped copy of the given vector
		(e.g. x-element of the vector will be clamped into the range [-amps.x, amps.x]);
		All elements of the amps should be greater than or equal zero
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector Clamp(const FVector& vector, const FVector& amps);


	static FORCEINLINE FVector inlToRadians(const FRotator& rot)
	{
		constexpr float DtR = PI / 180.0f;

		return FVector(DtR * rot.Roll, DtR * rot.Pitch, DtR * rot.Yaw);
	}

	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector ToRadians(const FRotator& rot);


	/*
		For the given angles returns Bn such that abs(Bn - A) <= abs(B - A).
		Expects angles in degrees within the range [-180, 180] 
	*/
	static FORCEINLINE float inlNearestAngle(float A, float B)
	{
		float delta{ B - A };
		float deltaRev{ delta + copysignf(360.0f, A) };

		if (fabsf(deltaRev) < fabsf(delta))
			return deltaRev + A;
		else
			return B;
	}

	/*
		For the given angles returns Bn such that abs(Bn - A) <= abs(B - A).
		Expects angles in degrees within the range [-180, 180] 
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float NearestAngle(float A, float B);


	static FORCEINLINE FVector inlCopySign(const FVector& target, const FVector& source)
	{
		return FVector(copysignf(target.X, source.X), copysignf(target.Y, source.Y), copysignf(target.Z, source.Z));
	}

	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector CopySign(const FVector& target, const FVector& source);


	UFUNCTION(BlueprintPure, Category = "XMath")
	static float CopySignf(float target, float source);


	static void FORCEINLINE inlSetSign(FVector& target, const FVector& source)
	{
		target.X = copysignf(target.X, source.X);
		target.Y = copysignf(target.Y, source.Y);
		target.Z = copysignf(target.Z, source.Z);
	}

	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void SetSign(UPARAM(ref) FVector& target, const FVector& source);


	/*
		Set the timing vector2D to: 
		X - is time to the moment of switching of the acceleration to inverse (from boost to retard),
		Y - is time to the moment of stopping at the specified distance;

		distance - distance in cm from current location to the desired location
		Vapp - approach speed in cm/s, if has the same sign as the distance then causes a decrease of the distance
		accel - desired value of boost and retard in cm/s^2, after this method call it matches the boost

		https://www.desmos.com/calculator/ylcswinfuy
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static void ConjugateTwoParabolas(float distance, float Vapp, UPARAM(ref) float& accel, FVector2D& timing);


	/**/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static void CTPVaryAcceleration(float distance, float Vapp, UPARAM(ref) float& accel, float tick, FVector2D& timing);


	/*
		Three angles overload of the method UXMath::ConjugateTwoParabolas(float distance, float Vapp, float accel);
		Returning vectors contain switching and stopping times for each angle, omega and epsilon from corresponding vectors;
		
		angles - Roll, Pitch, Yaw offsets in radians from desired attitude
		omegas - approach angular velocities in radians per second, if its element has the same sign as the corresponding angle offset,
		         then causes a decrease of this angle offset
		epsilons - desired angular accelerations of boost and retard in radians per second squared,
		           after this method call it matches the boost
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static void ConjugateTwoParabolas3A(const FVector& angles, const FVector& omegas, UPARAM(ref) FVector& epsilons, FVector& Tswitch, FVector& Tstop);


	/*
		
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float ForceOnSchedule(float Tswitch, float Tstop, float tick, float boost);


	/**/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector TorqueOnSchedule(const FVector& Tswitch, const FVector& Tstop, float tick, const FVector& boost);


	/*
		
		https://www.desmos.com/calculator/zadms3zfp1
		https://www.desmos.com/calculator/ffcliymj8h
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float BoostRetardController(float distance, float Vapp, float accel, float tick);


	/**/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector BoostRetardController3A(const FVector& angles, const FVector& omegas, UPARAM(ref) FVector& epsilons, float tick);


	template<typename T>
	static FORCEINLINE void LowPassFilter(const T& currentX, T& previousY, float alpha = 0.5f)
	{
		previousY += alpha * (currentX - previousY);
	};


	/*
		Discrete-time implementation of a simple RC low-pass filter.
		Returns currentY = previousY + alpha * (currentX - previuosY)

		currentX - current input value;
		previousY - previous output value, after call will be assigned to currentY;
		alpha - smoothing factor is chosen from the range [0, 1]
		close to 0 - strong smoothing
		close to 1 - slight smoothing
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void LowPassFilterFloat(float currentX, UPARAM(ref) float& previousY, float& currentY, float alpha = 0.5f);


	/*
		Discrete-time implementation of a simple RC low-pass filter.
		Returns currentY = previousY + alpha * (currentX - previuosY)

		currentX - current input value;
		previousY - previous output value, after call will be assigned to currentY;
		alpha - smoothing factor is chosen from the range [0, 1]
		close to 0 - strong smoothing
		close to 1 - slight smoothing
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void LowPassFilterVec(const FVector& currentX, UPARAM(ref) FVector& previousY, FVector& currentY, float alpha = 0.5f);

	/*
		Wrapper method for the FQuat::Slerp(FQuat current, FQuat target, float) which
		avoids the problem of gimbal lock
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XMath")
	static FRotator RInterpTo(const FRotator& current, const FRotator& target, float deltaTime, float interpSpeed);

	/*
		Interpolates between rotatations of the current and the target transforms
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static FTransform& TrotInterpTo(UPARAM(ref) FTransform& current, const FTransform& target, float deltaTime, float interpSpeed);

	/*
		Low-pass filter of a USceneComponent rotations.
		Does not take into account and does not affect physics.

		current - USceneComponent with smoothed rotations of the target
		alpha - smoothing factor is chosen from the range [0, 1]
		close to 0 - strong smoothing
		close to 1 - slight smoothing
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void ComponentRotLPF(UPARAM(ref) USceneComponent* current, const USceneComponent* target, float alpha);

	/*
		Maps given angle, expected in the range [-360, 360], to the range (-180, 180]. Angles are in degrees
	*/
	static FORCEINLINE float inlNormalizeClampedAngleD(float angle)
	{
		if (fabsf(angle) > 180.0f)
			angle -= copysignf(360.0f, angle);

		return angle;
	}

	/*
		Maps given angle, expected in the range [-360, 360], to the range (-180, 180]. Angles are in degrees
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float NormalizeClampedAngleD(float angle);
};
