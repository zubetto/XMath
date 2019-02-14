// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#include "XMath.h"



bool UXMath::Clampf(float input, float amplitude, float& output)
{
	return inlClampf(input, amplitude, output);
}

FVector& UXMath::ClampRef(FVector& vector, const FVector& amps)
{
	inlClamp(vector, amps);

	return vector;
}

FVector UXMath::Clamp(const FVector& vector, const FVector& amps)
{
	FVector copyV{ vector };
	inlClamp(copyV, amps);

	return copyV;
}

FVector UXMath::ToRadians(const FRotator& rot)
{
	return inlToRadians(rot);
}

float UXMath::NearestAngle(float A, float B)
{
	return inlNearestAngle(A, B);
}

FVector UXMath::CopySign(const FVector& target, const FVector& source)
{
	return inlCopySign(target, source);
}

float UXMath::CopySignf(float target, float source)
{
	return copysignf(target, source);
}

void UXMath::SetSign(FVector& target, const FVector& source)
{
	inlSetSign(target, source);
}

void UXMath::ConjugateTwoParabolas(float distance, float Vapp, float& accel, FVector2D& timing)
{
	accel = copysignf(accel, distance);

	float Tcomp{ Vapp / accel };

	float Tswitch{ sqrtf(0.5f * Tcomp * Tcomp + distance / accel) - Tcomp };

	// Set Tswitch and Tstop
	timing.X = Tswitch;
	timing.Y = 2.0f * Tswitch + Tcomp;
}

void UXMath::CTPVaryAcceleration(float distance, float Vapp, float& accel, float tick, FVector2D& timing)
{
	tick = fabsf(tick);

	// Desired time until the moment of switching from boost to retard
	// in case when Vapp is equal zero
	float Tswitch{ 2.0f * tick };

	if (signbit(distance) == signbit(Vapp)) // Then the acceleration is set equal to the distance times factor
	{
		// the factor has lower limit factorMin = 0.5 * (Vapp/distance)^2
		float factor{ Vapp / distance };
		factor *= 0.5 * factor;

		// factor = max(factorMin, 1.0/Tswitch^2)
		factor = fmaxf(factor, 1.0f / (Tswitch * Tswitch));

		accel = fminf(fabsf(accel), fabsf(factor * distance));
	}
	else // Then the acceleration is set to maintain the Tswitch invariable regardless of the distance
	{
		float deff{ 2.0f * Vapp * Tswitch - distance };
		float Tswsq{ 2.0f * Tswitch * Tswitch };
		float aopt{ sqrtf(deff * deff - Vapp * Vapp * Tswsq) };

		aopt = (copysignf(aopt, distance) - deff) / Tswsq;

		accel = fminf(fabsf(accel), fabsf(aopt));
	}

	accel = copysignf(accel, distance);

	float Tcomp{ Vapp / accel };

	// Actual Tswitch for the given accel:
	Tswitch = sqrtf(0.5f * Tcomp * Tcomp + distance / accel) - Tcomp;

	// Set Tswitch and Tstop
	timing.X = Tswitch;
	timing.Y = 2.0f * Tswitch + Tcomp;
}

void UXMath::ConjugateTwoParabolas3A(const FVector& angles, const FVector& omegas, FVector& epsilons, FVector& Tswitch, FVector& Tstop)
{
	SetSign(epsilons, angles);

	// --- ROLL --------------------------------------
	float Tcomp{ omegas.X / epsilons.X };

	Tswitch.X = sqrtf(0.5f * Tcomp * Tcomp + angles.X / epsilons.X) - Tcomp;
	Tstop.X = 2.0f * Tswitch.X + Tcomp;

	// --- PITCH -------------------------------------
	Tcomp = omegas.Y / epsilons.Y;

	Tswitch.Y = sqrtf(0.5f * Tcomp * Tcomp + angles.Y / epsilons.Y) - Tcomp;
	Tstop.Y = 2.0f * Tswitch.Y + Tcomp;

	// --- YAW ---------------------------------------
	Tcomp = omegas.Z / epsilons.Z;

	Tswitch.Z = sqrtf(0.5f * Tcomp * Tcomp + angles.Z / epsilons.Z) - Tcomp;
	Tstop.Z = 2.0f * Tswitch.Z + Tcomp;
}

float UXMath::ForceOnSchedule(float Tswitch, float Tstop, float tick, float boost)
{
	if (Tstop >= tick)
	{
		if (Tswitch > tick)
			return boost;
		else
			return -boost;
	}
	else
		return 0.0f;
}

FVector UXMath::TorqueOnSchedule(const FVector& Tswitch, const FVector& Tstop, float tick, const FVector& boost)
{
	FVector accels;

	// --- ROLL -----------------------------------------
	if (Tstop.X >= tick)
		accels.X = Tswitch.X > tick ? -boost.X : boost.X;
	else
		accels.X = 0.0f;

	// --- PITCH ----------------------------------------
	if (Tstop.Y >= tick)
		accels.Y = Tswitch.Y > tick ? -boost.Y : boost.Y;
	else
		accels.Y = 0.0f;

	// --- YAW ------------------------------------------
	if (Tstop.Z >= tick)
		accels.Z = Tswitch.Z > tick ? boost.Z : -boost.Z;
	else
		accels.Z = 0.0f;

	return accels;
}

float UXMath::BoostRetardController(float distance, float Vapp, float accel, float tick)
{
	FVector2D timing;

	CTPVaryAcceleration(distance, Vapp, accel, tick, timing);

	return ForceOnSchedule(timing.X, timing.Y, tick, accel);
}

FVector UXMath::BoostRetardController3A(const FVector& angles, const FVector& omegas, UPARAM(ref)FVector& epsilons, float tick)
{
	FVector2D timing;
	FVector Tswitch;
	FVector Tstop;

	CTPVaryAcceleration(angles.X, omegas.X, epsilons.X, tick, timing);
	Tswitch.X = timing.X;
	Tstop.X = timing.Y;

	CTPVaryAcceleration(angles.Y, omegas.Y, epsilons.Y, tick, timing);
	Tswitch.Y = timing.X;
	Tstop.Y = timing.Y;

	CTPVaryAcceleration(angles.Z, omegas.Z, epsilons.Z, tick, timing);
	Tswitch.Z = timing.X;
	Tstop.Z = timing.Y;

	return TorqueOnSchedule(Tswitch, Tstop, tick, epsilons);
}

void UXMath::LowPassFilterFloat(float currentX, UPARAM(ref) float& previousY, float& currentY, float alpha)
{
	UXMath::LowPassFilter<float>(currentX, previousY, alpha);
	currentY = previousY;
}

void UXMath::LowPassFilterVec(const FVector& currentX, UPARAM(ref)FVector& previousY, FVector& currentY, float alpha)
{
	UXMath::LowPassFilter<FVector>(currentX, previousY, alpha);
	currentY = previousY;
}

FRotator UXMath::RInterpTo(const FRotator& current, const FRotator& target, float deltaTime, float interpSpeed)
{
	float alpha{ deltaTime * interpSpeed };

	if (alpha <= 0.0)
	{
		return current;
	}
	else if (alpha >= 1.0)
	{
		return target;
	}
	else
	{
		return FRotator(FQuat::Slerp(FQuat(current), FQuat(target), alpha));
	}
}

FTransform& UXMath::TrotInterpTo(FTransform& current, const FTransform& target, float deltaTime, float interpSpeed)
{
	float alpha{ deltaTime * interpSpeed };

	if (alpha <= 0.0)
	{
		return current;
	}
	else if (alpha >= 1.0)
	{
		current = target;
		return current;
	}
	else
	{
		current.SetRotation(FQuat::Slerp(current.GetRotation(), target.GetRotation(), alpha));
		return current;
	}
}

void UXMath::ComponentRotLPF(USceneComponent* current, const USceneComponent* target, float alpha)
{
	if (alpha <= 0.0)
	{
		return;
	}
	else if (alpha >= 1.0)
	{
		current->SetWorldRotation(target->GetComponentQuat(), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		FQuat newQuat{ FQuat::Slerp(current->GetComponentQuat(), target->GetComponentQuat(), alpha) };
		current->SetWorldRotation(newQuat, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

float UXMath::NormalizeClampedAngleD(float angle)
{
	return inlNormalizeClampedAngleD(angle);
}
