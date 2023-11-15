#pragma once

namespace engine
{

class ICameraController
{
public:
	ICameraController() = default;
	ICameraController(const ICameraController&) = default;
	ICameraController(ICameraController&&) = default;
	ICameraController& operator=(const ICameraController&) = default;
	ICameraController& operator=(ICameraController&&) = default;
	virtual ~ICameraController() = default;

	// Operations
	virtual bool IsInAnimation() const { return false; }
	virtual bool IsZooming() const { return false; }
	virtual bool IsFOVZooming() const { return false; }
	virtual bool IsPanning() const { return false; }
	virtual bool IsTurning() const { return false; }
	virtual bool IsTracking() const { return false; }
	virtual bool IsInWalkMode() const { return false; }
	virtual bool IsInControl() const { return IsInAnimation() || IsZooming() || IsFOVZooming() || IsPanning() || IsTurning() || IsTracking() || IsInWalkMode(); }

	// Event Handlers
	virtual void OnMouseDown() {}
	virtual void OnMouseUp() {}
	virtual void OnMouseMove(float x, float y) {}
	virtual void OnMouseWheel(float y) {}
};

}