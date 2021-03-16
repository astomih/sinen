#pragma once
#include <cstdint>
#include "MoveComponent.h"

class InputComponent : public MoveComponent
{
public:
	// Lower update order to update first
	InputComponent(class Actor& owner);

	void ProcessInput(const struct InputState& state) override;

	// Getters/setters for private variables
	float GetMaxForward() const { return mMaxForwardSpeed; }
	float GetMaxAngular() const { return mMaxAngularSpeed; }
	int GetForwardKey() const { return mForwardKey; }
	int GetBackKey() const { return mBackKey; }
	int GetRightMoveKey() const { return mRightMoveKey; }
	int GetLeftMoveKey() const { return mLeftMoveKey; }
	int GetClockwiseKey() const { return mClockwiseKey; }
	int GetCounterClockwiseKey() const { return mCounterClockwiseKey; }

	void SetMaxForwardSpeed(float speed) { mMaxForwardSpeed = speed; }
	void SetMaxAngularSpeed(float speed) { mMaxAngularSpeed = speed; }
	void SetMaxLeftRightSpeed(float speed) { mMaxLeftRightSpeed = speed; }
	void SetForwardKey(int key) { mForwardKey = key; }
	void SetBackKey(int key) { mBackKey = key; }
	void SetRightMoveKey(int key) { mRightMoveKey = key; }
	void SetLeftMoveKey(int key) { mLeftMoveKey = key; }
	void SetClockwiseKey(int key) { mClockwiseKey = key; }
	void SetCounterClockwiseKey(int key) { mCounterClockwiseKey = key; }

	void SetDirectionMode(bool Use) { mUseDirectionMode = Use; }
private:
	// The maximum forward/angular speeds
	float mMaxForwardSpeed = 0;
	float mMaxLeftRightSpeed = 0;
	float mMaxAngularSpeed = 0;
	// Keys for movement
	int mForwardKey = 0;
	int mBackKey = 0;
	int mRightMoveKey = 0;
	int mLeftMoveKey = 0;
	// Keys for angular movement
	int mClockwiseKey = 0;
	int mCounterClockwiseKey = 0;

	bool mUseDirectionMode = true;
};
