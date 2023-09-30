#include "Collision.hpp"

#include "BedrockAssert.hpp"
#include "BedrockMath.hpp"
#include "JobSystem.hpp"

#include <algorithm>
#include <set>
#include <future>

namespace MFA::Collision
{

	//-------------------------------------------------------------------------------------------------

	bool HasIntersection(
		Triangle const& triangle,
		glm::dvec3 const& currentPos_,
		glm::dvec3 const& previousPos_,
		glm::dvec3& outCollisionPos,
		double& outTime,
		double const epsilon,
		bool const checkForBackCollision
	)
	{
		auto currentPos = currentPos_;
		auto previousPos = previousPos_;

		auto const movementVector = currentPos - previousPos;

		auto movementMagnitude = glm::length(movementVector);

		if (movementMagnitude == 0.0)
		{
			return false;
		}

		auto const movementDirection = movementVector / movementMagnitude;

		previousPos -= movementDirection * epsilon;
		currentPos += movementDirection * epsilon;
		movementMagnitude += 2 * epsilon;

		auto const surfaceDot2 = glm::dot(previousPos - triangle.center, triangle.normal);
		auto const surfaceDir2 = surfaceDot2 >= 0.0 ? 1.0f : -1.0f;

		if (checkForBackCollision == false && surfaceDir2 < 0)
		{
			return false;
		}

		auto const surfaceDot1 = glm::dot(currentPos - triangle.center, triangle.normal);
		auto const surfaceDir1 = surfaceDot1 >= 0.0 ? 1.0f : -1.0f;

		if (surfaceDir1 == surfaceDir2)
		{
			return false;
		}

		auto const bottom = glm::dot(movementDirection, triangle.normal);

		if (std::abs(bottom) == 0.0)
		{
			return false;
		}

		auto const top = glm::dot(-previousPos + triangle.center, triangle.normal);

		auto const time = top / bottom;

		if (time < 0.0 || time > movementMagnitude)
		{
			return false;
		}

		outCollisionPos = previousPos + time * movementDirection;

		if (IsInsideTriangle(triangle, outCollisionPos) == false)
		{
			return false;
		}

		outTime = time;

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasIntersection(
		Triangle const& triangle,
		glm::dvec3 const& currentPos,
		glm::dvec3 const& previousPos,
		glm::dvec3& outCollisionPos,
		double const epsilon,
		bool const checkForBackCollision
	)
	{
		double outTime = 0.0;
		return HasIntersection(
			triangle,
			currentPos,
			previousPos,
			outCollisionPos,
			outTime,
			epsilon,
			checkForBackCollision
		);
	}

	//-------------------------------------------------------------------------------------------------

	bool HasDynamicIntersection(
		glm::dvec3 const& pointPrevPos, glm::dvec3 const& pointCurrPos,
		glm::dvec3 const& triPrevPos0, glm::dvec3 const& triPrevPos1, glm::dvec3 const& triPrevPos2,
		glm::dvec3 const& triCurrPos0, glm::dvec3 const& triCurrPos1, glm::dvec3 const& triCurrPos2,
		double epsilon, bool checkForBackCollision
	)
	{
		glm::dvec3 localPrevPoint{};
		glm::dvec3 localPrevTri0{};
		glm::dvec3 localPrevTri1{};
		glm::dvec3 localPrevTri2{};
		{// Local coordinate for previous state
			glm::dvec3 prevCoordCenter{};
			glm::dvec3 prevCoordBiTan{};
			glm::dvec3 prevCoordNormal{};
			glm::dvec3 prevCoordTan{};

			Math::GenerateLocalCoordinate(
				triPrevPos0, triPrevPos1, triPrevPos2,
				prevCoordCenter, prevCoordBiTan, prevCoordNormal, prevCoordTan
			);

			localPrevPoint = Math::ToLocalCoordinate(
				pointPrevPos,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);

			localPrevTri0 = Math::ToLocalCoordinate(
				triPrevPos0,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);

			localPrevTri1 = Math::ToLocalCoordinate(
				triPrevPos1,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);

			localPrevTri2 = Math::ToLocalCoordinate(
				triPrevPos2,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);
		}

		glm::dvec3 localCurrPoint {};
		glm::dvec3 localCurrTri0{};
		glm::dvec3 localCurrTri1{};
		glm::dvec3 localCurrTri2{};
		{// Local coordinate for next state
			glm::dvec3 currCoordCenter{};
			glm::dvec3 currCoordBiTan{};
			glm::dvec3 currCoordNormal{};
			glm::dvec3 currCoordTan{};
			// TODO: We can cache local coordinate
			Math::GenerateLocalCoordinate(
				triCurrPos0, triCurrPos1, triCurrPos2,
				currCoordCenter, currCoordBiTan, currCoordNormal, currCoordTan
			);

			localCurrPoint = Math::ToLocalCoordinate(
				pointCurrPos,
				currCoordCenter,
				currCoordBiTan,
				currCoordNormal,
				currCoordTan
			);

			localCurrTri0 = Math::ToLocalCoordinate(
				triCurrPos0,
				currCoordCenter,
				currCoordBiTan,
				currCoordNormal,
				currCoordTan
			);

			localCurrTri1 = Math::ToLocalCoordinate(
				triCurrPos1,
				currCoordCenter,
				currCoordBiTan,
				currCoordNormal,
				currCoordTan
			);

			localCurrTri2 = Math::ToLocalCoordinate(
				triCurrPos2,
				currCoordCenter,
				currCoordBiTan,
				currCoordNormal,
				currCoordTan
			);
		}

		glm::dvec3 collisionPos {};
		double collisionTime {};

		{
			auto const triangleCenter = (localPrevTri0 + localPrevTri1 + localPrevTri2) / 3.0;
			auto const triangleNormal = glm::normalize(glm::cross(localPrevTri1 - localPrevTri0, localPrevTri2 - localPrevTri1));

			auto currentPos = localCurrPoint;
			auto previousPos = localPrevPoint;

			auto const movementVector = currentPos - previousPos;

			auto movementMagnitude = glm::length(movementVector);

			if (movementMagnitude == 0.0)
			{
				return false;
			}

			auto const movementDirection = movementVector / movementMagnitude;

			previousPos -= movementDirection * epsilon;
			currentPos += movementDirection * epsilon;
			movementMagnitude += 2 * epsilon;

			// I think this just causes issue and does not help in term of performance either
			/*auto const surfaceDot2 = glm::dot(previousPos - triangleCenter, triangleNormal);
			auto const surfaceDir2 = surfaceDot2 >= 0.0 ? 1.0f : -1.0f;

			if (checkForBackCollision == false && surfaceDir2 < 0)
			{
				return false;
			}

			auto const surfaceDot1 = glm::dot(currentPos - triangleCenter, triangleNormal);
			auto const surfaceDir1 = surfaceDot1 >= 0.0 ? 1.0f : -1.0f;

			if (surfaceDir1 == surfaceDir2)
			{
				return false;
			}*/

			auto const bottom = glm::dot(movementDirection, triangleNormal);

			if (std::abs(bottom) == 0.0)
			{
				return false;
			}

			auto const top = glm::dot(-previousPos + triangleCenter, triangleNormal);

			auto const time = top / bottom;

			if (time < 0.0 || time > movementMagnitude)
			{
				return false;
			}

			collisionPos = previousPos + time * movementDirection;
			collisionTime = time / movementMagnitude;
		}

		auto const collisionTriangle = GenerateCollisionTriangle(
			glm::mix(localPrevTri0, localCurrTri0, collisionTime), 
			glm::mix(localPrevTri1, localCurrTri1, collisionTime), 
			glm::mix(localPrevTri2, localCurrTri2, collisionTime)
		);

		return IsInsideTriangle(collisionTriangle, collisionPos);
	}

	//-------------------------------------------------------------------------------------------------

	bool HasDynamicEdgeIntersection(
		glm::dvec3 const& p0Prev, 
		glm::dvec3 const& p1Prev, 
		glm::dvec3 const& p0Curr,
		glm::dvec3 const& p1Curr, 
		glm::dvec3 const& q0Prev, 
		glm::dvec3 const& q1Prev, 
		glm::dvec3 const& q0Curr,
		glm::dvec3 const& q1Curr,

		double epsilon,
		double & collisionTime,
		bool checkForSegmentIntersection
	)
	{
		auto const& pointPrevPos = p0Prev;
		auto const& pointCurrPos = p0Curr;

		auto const& triPrevPos0 = p1Prev;
		auto const& triCurrPos0 = p1Curr;
		auto const& triPrevPos1 = q0Prev;
		auto const& triCurrPos1 = q0Curr;
		auto const& triPrevPos2 = q1Prev;
		auto const& triCurrPos2 = q1Curr;

		glm::dvec3 localPrevPoint{};
		glm::dvec3 localPrevTri0{};
		glm::dvec3 localPrevTri1{};
		glm::dvec3 localPrevTri2{};
		{// Local coordinate for previous state
			glm::dvec3 prevCoordCenter{};
			glm::dvec3 prevCoordBiTan{};
			glm::dvec3 prevCoordNormal{};
			glm::dvec3 prevCoordTan{};

			Math::GenerateLocalCoordinate(
				triPrevPos0, triPrevPos1, triPrevPos2,
				prevCoordCenter, prevCoordBiTan, prevCoordNormal, prevCoordTan
			);

			localPrevPoint = Math::ToLocalCoordinate(
				pointPrevPos,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);

			localPrevTri0 = Math::ToLocalCoordinate(
				triPrevPos0,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);

			localPrevTri1 = Math::ToLocalCoordinate(
				triPrevPos1,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);

			localPrevTri2 = Math::ToLocalCoordinate(
				triPrevPos2,
				prevCoordCenter,
				prevCoordBiTan,
				prevCoordNormal,
				prevCoordTan
			);
		}

		glm::dvec3 localCurrPoint{};
		{// Local coordinate for next state
			glm::dvec3 currCoordCenter{};
			glm::dvec3 currCoordBiTan{};
			glm::dvec3 currCoordNormal{};
			glm::dvec3 currCoordTan{};
			// TODO: We can cache local coordinate
			Math::GenerateLocalCoordinate(
				triCurrPos0, triCurrPos1, triCurrPos2,
				currCoordCenter, currCoordBiTan, currCoordNormal, currCoordTan
			);

			localCurrPoint = Math::ToLocalCoordinate(
				pointCurrPos,
				currCoordCenter,
				currCoordBiTan,
				currCoordNormal,
				currCoordTan
			);
		}

		collisionTime = {};

		{
			auto const triangleCenter = (localPrevTri0 + localPrevTri1 + localPrevTri2) / 3.0;
			auto const triangleNormal = glm::normalize(glm::cross(localPrevTri1 - localPrevTri0, localPrevTri2 - localPrevTri1));

			auto currentPos = localCurrPoint;
			auto previousPos = localPrevPoint;

			auto const movementVector = currentPos - previousPos;

			auto movementMagnitude = glm::length(movementVector);

			if (movementMagnitude == 0.0)
			{
				return false;
			}

			auto const movementDirection = movementVector / movementMagnitude;

			previousPos -= movementDirection * epsilon;
			currentPos += movementDirection * epsilon;
			movementMagnitude += 2 * epsilon;

			// I think this just causes issue and does not help in term of performance either
			//auto const surfaceDot2 = glm::dot(previousPos - triangleCenter, triangleNormal);
			//auto const surfaceDir2 = surfaceDot2 >= 0.0 ? 1.0f : -1.0f;

			//auto const surfaceDot1 = glm::dot(currentPos - triangleCenter, triangleNormal);
			//auto const surfaceDir1 = surfaceDot1 >= 0.0 ? 1.0f : -1.0f;

			/*if (surfaceDir1 == surfaceDir2)
			{
				return false;
			}*/

			auto const bottom = glm::dot(movementDirection, triangleNormal);

			if (std::abs(bottom) == 0.0)
			{
				return false;
			}

			auto const top = glm::dot(-previousPos + triangleCenter, triangleNormal);

			auto const time = top / bottom;

			if (time < 0.0 || time > movementMagnitude)
			{
				return false;
			}

			collisionTime = time / movementMagnitude;
		}

		if (checkForSegmentIntersection == false)
		{
			return true;
		}

		auto const p0ColPos = glm::mix(p0Prev, p0Curr, collisionTime);
		auto const p1ColPos = glm::mix(p1Prev, p1Curr, collisionTime);

		auto const q0ColPos = glm::mix(q0Prev, q0Curr, collisionTime);
		auto const q1ColPos = glm::mix(q1Prev, q1Curr, collisionTime);

		auto const pDir = glm::normalize(p1ColPos - p0ColPos);
		auto const qDir = glm::normalize(q1ColPos - q0ColPos);

		return DoSegmentsIntersect(
			p0ColPos - pDir * epsilon, 
			p1ColPos + pDir * epsilon, 
			q0ColPos - qDir * epsilon, 
			q1ColPos + qDir * epsilon
		);
	}

	//-------------------------------------------------------------------------------------------------

	bool IsInside(std::vector<Triangle> const& triangles, glm::dvec3 const& point)
	{
		glm::dvec3 const outsidePos = point + Math::DRightVec3 * 1000.0;

		int intersectionCount = 0;

		for (auto const& triangle : triangles)
		{
			glm::dvec3 collisionPos{};
			if (HasIntersection(triangle, outsidePos, point, collisionPos, 0.0, true) == true)
			{
				++intersectionCount;
			}
		}
		// Even number of collision means that there is no collision
		if (intersectionCount % 2 == 0)
		{
			return false;
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool IsInside(StaticTriangleGrid& grid, glm::dvec3 const& point)
	{
		glm::dvec3 const outsidePos = point + Math::DRightVec3 * 1000.0;

		int intersectionCount = 0;

		for (auto const& triangle : grid.GetNearbyTriangles(point, outsidePos))
		{
			glm::dvec3 collisionPos{};
			if (HasIntersection(*triangle, outsidePos, point, collisionPos, 0.0, true) == true)
			{
				++intersectionCount;
			}
		}
		// Even number of collision means that there is no collision
		if (intersectionCount % 2 == 0)
		{
			return false;
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool FindClosestTriangle(
		std::vector<Triangle> const& triangles,
		glm::dvec3 const& point,
		int& outTriangleIdx,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	)
	{
		double leastDist = -1.0;
		Triangle const* closestTriangle{};

		// Searching for nearest triangle
		for (int i = 0; i < static_cast<int>(triangles.size()); ++i)
		{
			auto& triangle = triangles[i];

			double distance = 0.0;
			glm::dvec3 planePosition{};

			bool isValid = CalcDistanceToTriangleFast(
				triangle,
				point,
				distance,
				planePosition
			);

			if (isValid == true)
			{
				if (leastDist == -1.0 || distance < leastDist)
				{
					leastDist = distance;
					closestTriangle = &triangle;
					outTrianglePosition = planePosition;
					outTriangleNormal = triangle.normal;
					outTriangleIdx = i;
				}
			}
		}

		if (closestTriangle == nullptr)
		{
			return false;
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasContiniousCollision(
		std::vector<Triangle>& triangles,
		glm::dvec3 const& prevPos,
		glm::dvec3 const& nextPos,
		int& outTriangleIdx,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	)
	{
		int collisionCount = 0;
		double leastTime = -1.0;
		// We need to choose closest triangle
		for (int i = 0; i < triangles.size(); ++i)
		{
			auto const& triangle = triangles[i];

			glm::dvec3 collisionPos{};
			double time = 0.0;

			if (HasIntersection(
				triangle,
				nextPos,
				prevPos,
				collisionPos,
				time,
				0.0,
				false
			))
			{
				++collisionCount;
				if (leastTime == -1.0 || time < leastTime)
				{
					leastTime = time;
					outTriangleIdx = i;
					outTriangleNormal = triangle.normal;
					outTrianglePosition = collisionPos;
				}
			}
		}

		return leastTime != -1.0;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasContiniousCollision(
		StaticTriangleGrid& grid,
		glm::dvec3 const& prevPos,
		glm::dvec3 const& nextPos,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	)
	{
		// We need to choose closest triangle
		auto const triangles = grid.GetNearbyTriangles(prevPos, nextPos);
		int collisionCount = 0;
		double leastTime = -1.0;
		
		for (int i = 0; i < triangles.size(); ++i)
		{
			auto const& triangle = triangles[i];

			glm::dvec3 collisionPos{};
			double time = 0.0;

			if (Collision::HasIntersection(
				*triangle,
				nextPos,
				prevPos,
				collisionPos,
				time,
				0.0,
				false
			))
			{
				++collisionCount;
				if (leastTime == -1.0 || time < leastTime)
				{
					leastTime = time;
					outTriangleNormal = triangle->normal;
					outTrianglePosition = collisionPos;
				}
			}
		}

		return leastTime != -1.0;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasContiniousCollision(
		std::vector<Triangle const*>& triangles, 
		glm::dvec3 const& prevPos,
		glm::dvec3 const& nextPos, 
		int& outTriangleIdx, 
		glm::dvec3& outTrianglePosition, 
		glm::dvec3& outTriangleNormal,
		double epsilon
	)
	{
		int collisionCount = 0;
		double leastTime = -1.0;
		outTriangleIdx = -1;

		for (int i = 0; i < triangles.size(); ++i)
		{
			auto const& triangle = triangles[i];

			glm::dvec3 collisionPos{};
			double time = 0.0;

			if (HasIntersection(
				*triangle,
				nextPos,
				prevPos,
				collisionPos,
				time,
				epsilon,
				false
			))
			{
				++collisionCount;
				if (leastTime == -1.0 || time < leastTime)
				{
					outTriangleIdx = i;
					leastTime = time;
					outTriangleNormal = triangle->normal;
					outTrianglePosition = collisionPos;
				}
			}
		}

		return leastTime != -1.0;
	}

	//-------------------------------------------------------------------------------------------------

	bool FindClosestTriangle(
		StaticTriangleGrid& grid,
		glm::dvec3 const& point,
		int& outTriangleIdx,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	)
	{
		double leastSqrDist = -1.0;
		Triangle const* closestTriangle{};

		auto triangles = grid.GetNearbyTriangles(point);
		if (triangles.empty())
		{
			MFA_LOG_WARN("Failed to retreive triangles from grid");
			return FindClosestTriangle(
				grid.GetTriangles(),
				point,
				outTriangleIdx,
				outTrianglePosition,
				outTriangleNormal
			);
		}

		// Searching for nearest triangle
		for (int i = 0; i < static_cast<int>(triangles.size()); ++i)
		{
			auto& triangle = triangles[i];

			double sqrDistance = 0.0;
			glm::dvec3 planePosition{};

			auto const isValid = CalcDistanceToTriangleFast(
				*triangle,
				point,
				sqrDistance,
				planePosition
			);

			if (isValid == true)
			{
				if (leastSqrDist == -1.0 || sqrDistance < leastSqrDist)
				{
					leastSqrDist = sqrDistance;
					closestTriangle = triangle;
					outTrianglePosition = planePosition;
					outTriangleNormal = triangle->normal;
					outTriangleIdx = i;
				}
			}
		}

		if (closestTriangle == nullptr)
		{
			return false;
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool FindClosestTriangle(
		std::vector<Triangle const*> const& triangles, 
		glm::dvec3 const& point,
		int& outTriangleIdx, 
		glm::dvec3& outTrianglePosition, 
		glm::dvec3& outTriangleNormal
	)
	{
		double leastDist = -1.0;
		Triangle const* closestTriangle{};

		// Searching for nearest triangle
		for (int i = 0; i < static_cast<int>(triangles.size()); ++i)
		{
			auto& triangle = triangles[i];

			double distance = 0.0;
			glm::dvec3 planePosition{};

			auto const isValid = CalcDistanceToTriangleFast(
				*triangle,
				point,
				distance,
				planePosition
			);

			if (isValid == true)
			{
				if (leastDist == -1.0 || distance < leastDist)
				{
					leastDist = distance;
					closestTriangle = triangle;
					outTrianglePosition = planePosition;
					outTriangleNormal = triangle->normal;
					outTriangleIdx = i;
				}
			}
		}

		if (closestTriangle == nullptr)
		{
			return false;
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasStaticCollision(
		const std::vector<Triangle>& triangles,
		glm::dvec3 const& point,
		int& outTriangleIdx,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	)
	{
		if (IsInside(triangles, point) == false)
		{
			return false;
		}

		return FindClosestTriangle(
			triangles,
			point,
			outTriangleIdx,
			outTrianglePosition,
			outTriangleNormal
		);
	}

	//-------------------------------------------------------------------------------------------------

	bool HasStaticCollision(
		StaticTriangleGrid& grid,
		glm::dvec3 const& point,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	)
	{
		if (IsInside(grid, point) == false)
		{
			return false;
		}

		auto resolutionTriangles = grid.GetNearbyTriangles(point);

		int triangleIdx{};

		return FindClosestTriangle(
			grid,
			point,
			triangleIdx,
			outTrianglePosition,
			outTriangleNormal
		);
	}

	//-------------------------------------------------------------------------------------------------

	bool HasSelfCollision(
		const std::vector<Triangle>& triangles,
		glm::dvec3 const& point,
		double const thickness,
		int& outTriangleIdx
	)
	{
		double leastDistance = -1.0;
		glm::dvec3 planePosition{};

		for (int i = 0; i < triangles.size(); ++i)
		{
			double distance{};
			CalcDistanceToTriangle(triangles[i], point, distance, planePosition);
			if (distance < thickness && (leastDistance == -1.0f || distance < leastDistance))
			{
				outTriangleIdx = i;
				leastDistance = distance;
			}
		}

		return leastDistance != -1.0;
	}

	//-------------------------------------------------------------------------------------------------

	// Point must be on the triangle plane
	[[nodiscard]]
	int FindClosestEdge(glm::dvec3 const& point, Triangle const& triangle)
	{
		double distance0{};
		auto const isDist0Valid = CalcSqrDistanceToEdge(
			triangle.edgeVertices[0],
			triangle.edgeVertices[1],
			point,
			distance0
		);

		double distance1{};
		auto const isDist1Valid = CalcSqrDistanceToEdge(
			triangle.edgeVertices[1],
			triangle.edgeVertices[2],
			point,
			distance1
		);

		double distance2{};
		auto const isDist2Valid = CalcSqrDistanceToEdge(
			triangle.edgeVertices[2],
			triangle.edgeVertices[0],
			point,
			distance2
		);

		int score = 0;

		if (isDist0Valid == true)
		{
			score += 1;
		}
		if (isDist1Valid == true)
		{
			score += 2;
		}
		if (isDist2Valid == true)
		{
			score += 4;
		}

		switch (score)
		{
		case 0:
			MFA_ASSERT(false);
			break;
		case 1:
			return 0;
		case 2:
			return 1;
		case 3:
			if (distance0 < distance1)
			{
				return 0;
			}
			return 1;
		case 4:
			return 2;
		case 5:
			if (distance0 < distance2)
			{
				return 0;
			}
			return 2;
		case 6:
			if (distance1 < distance2)
			{
				return 1;
			}
			return 2;
		case 7:
			if (distance0 < distance1)
			{
				if (distance0 < distance2)
				{
					return 0;
				}
				return 2;
			}
			if (distance1 < distance2)
			{
				return 1;
			}
			return 2;
		}

		return -1;
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Triangle const*> HasDiscreteEdgeCollision(
		StaticTriangleGrid const& grid,
		glm::dvec3 const& myEdge0,
		glm::dvec3 const& myEdge1
	)
	{
		std::vector<Triangle const*> collidedTriangles{};

		auto const triangles = grid.GetNearbyTriangles(myEdge0, myEdge1);
		for (auto const& triangle : triangles)
		{
			glm::dvec3 intersectionPoint{};
			if (HasIntersection(*triangle, myEdge0, myEdge1, intersectionPoint, 0.0, true))
			{
				collidedTriangles.emplace_back(triangle);
			}
		}
		return collidedTriangles;
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Triangle const*> HasDiscreteEdgeCollision(
		const std::vector<Triangle>& triangles,
		glm::dvec3 const& myEdge0,
		glm::dvec3 const& myEdge1
	)
	{
		std::vector<Triangle const*> collidedTriangles{};

		for (auto const& triangle : triangles)
		{
			glm::dvec3 intersectionPoint{};
			if (HasIntersection(triangle, myEdge0, myEdge1, intersectionPoint, 0.0, true))
			{
				collidedTriangles.emplace_back(&triangle);
			}
		}

		return collidedTriangles;
	}

	//-------------------------------------------------------------------------------------------------

	bool DoSegmentsIntersect(
		glm::dvec3 const& p0_3d,
		glm::dvec3 const& p1_3d,
		glm::dvec3 const& q0_3d,
		glm::dvec3 const& q1_3d,

		bool debug
	)
	{
		auto const p2p1Diff = p1_3d - p0_3d;
		auto const p2p1Len = glm::length(p2p1Diff);
		auto const p2p1Vec = p2p1Diff / p2p1Len;

		auto const q2q1Diff = q1_3d - q0_3d;
		auto const q2q1Len = glm::length(q2q1Diff);
		auto const q2q1Vec = q2q1Diff / q2q1Len;

		static constexpr double epsilon = 1e-5;

		// TODO: Try length2 for better performance
		auto const IsInsideSegment = [](
			glm::dvec3 const & point, 
			glm::dvec3 const & segment0, 
			glm::dvec3 const& segment1, 
			double segmentLength
		)
		{
			// TODO: Try length2 for better performance
			auto const pointDistance = glm::length(point - segment0) + glm::length(point - segment1);
			return pointDistance <= segmentLength + epsilon;
		};

		if (IsInsideSegment(p1_3d, q1_3d, q0_3d, q2q1Len))
		{
			if (debug == true)
			{
				MFA_LOG_INFO("Success: Segments vertices intersect");
			}
			return true;
		}

		if (IsInsideSegment(p0_3d, q1_3d, q0_3d, q2q1Len))
		{
			if (debug == true)
			{
				MFA_LOG_INFO("Success: Segments vertices intersect");
			}
			return true;
		}

		if (IsInsideSegment(q0_3d, p1_3d, p0_3d, p2p1Len))
		{
			if (debug == true)
			{
				MFA_LOG_INFO("Success: Segments vertices intersect");
			}
			return true;
		}

		if (IsInsideSegment(q1_3d, p1_3d, p0_3d, p2p1Len))
		{
			if (debug == true)
			{
				MFA_LOG_INFO("Success: Segments vertices intersect");
			}
			return true;
		}

		auto const normal = glm::cross(p2p1Vec, q2q1Vec);

		auto const normalMag = glm::length(normal);
		// Check if lines are parallel
		if (normalMag < glm::epsilon<double>())
		{
			if (debug == true)
			{
				MFA_LOG_INFO("Failed: Normal is very low");
			}
			return false;
		}

		int q0_dotDir = 0;
		int q1_dotDir = 0;
		{
			auto const pN = glm::cross(p2p1Vec, normal);
			auto const dot0 = glm::dot(q0_3d - p0_3d, pN);
			auto const dot1 = glm::dot(q1_3d - p0_3d, pN);

			q0_dotDir = dot0 >= 0.0 ? 1 : -1;
			q1_dotDir = dot1 >= 0.0 ? 1 : -1;

			if (q0_dotDir == q1_dotDir)
			{
				if (debug == true)
				{
					MFA_LOG_INFO("Failed: Same direction");
				}
				return false;
			}
		}

		int p0_dotDir = 0;
		int p1_dotDir = 0;
		{
			auto const qN = glm::cross(q2q1Vec, normal);
			auto const dot0 = glm::dot(p0_3d - q0_3d, qN);
			auto const dot1 = glm::dot(p1_3d - q0_3d, qN);

			p0_dotDir = dot0 >= 0.0 ? 1 : -1;
			p1_dotDir = dot1 >= 0.0 ? 1 : -1;

			if (p0_dotDir == p1_dotDir)
			{
				if (debug == true)
				{
					MFA_LOG_INFO("Failed: Same direction");
				}
				return false;
			}
		}

		if (debug == true)
		{
			MFA_LOG_INFO("Success: Different directions");
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasContiniousEdgeCollision(
		double deltaTime,

		StaticTriangleGrid const& grid,

		glm::dvec3 const& p0PrevPos,
		glm::dvec3 const& p0NextPos,
		glm::dvec3 const& p1PrevPos,
		glm::dvec3 const& p1NextPos,

		std::vector<glm::dvec3>& planeNormal,
		std::vector<glm::dvec3>& planePosition,
		std::vector<glm::dvec3>& p0ColPos,
		std::vector<glm::dvec3>& p1ColPos,

		bool debug
	)
	{
		MFA_NOT_IMPLEMENTED_YET("M F");
		return false;
	}

	//-------------------------------------------------------------------------------------------------

	bool HasContiniousEdgeCollision(
		double const deltaTime,

		const std::vector<Triangle>& triangles,

		glm::dvec3 const& p0PrevPos,
		glm::dvec3 const& p0NextPos,
		glm::dvec3 const& p1PrevPos,
		glm::dvec3 const& p1NextPos,

		std::vector<glm::dvec3>& planeNormals,
		std::vector<glm::dvec3>& planePositions,
		std::vector<glm::dvec3>& p0ColPoses,
		std::vector<glm::dvec3>& p1ColPoses,

		bool debug
	)
	{
		for (auto const& triangle : triangles)
		{
			{
				glm::dvec3 planeNormal{};
				glm::dvec3 planePosition{};
				glm::dvec3 p0ColPos{};
				glm::dvec3 p1ColPos{};

				auto hasCollision = HasContiniousEdgeCollision(
					deltaTime,
					triangle.edgeVertices[0],
					triangle.edgeVertices[1],
					p0PrevPos,
					p0NextPos,
					p1PrevPos,
					p1NextPos,
					planeNormal,
					planePosition,
					p0ColPos,
					p1ColPos,
					debug
				);

				if (hasCollision == true)
				{
					planeNormals.emplace_back(planeNormal);
					planePositions.emplace_back(planePosition);
					p0ColPoses.emplace_back(p0ColPos);
					p1ColPoses.emplace_back(p1ColPos);
				}
			}

			{
				glm::dvec3 planeNormal{};
				glm::dvec3 planePosition{};
				glm::dvec3 p0ColPos{};
				glm::dvec3 p1ColPos{};

				auto hasCollision = HasContiniousEdgeCollision(
					deltaTime,
					triangle.edgeVertices[1],
					triangle.edgeVertices[2],
					p0PrevPos,
					p0NextPos,
					p1PrevPos,
					p1NextPos,
					planeNormal,
					planePosition,
					p0ColPos,
					p1ColPos,
					debug
				);

				if (hasCollision == true)
				{
					planeNormals.emplace_back(planeNormal);
					planePositions.emplace_back(planePosition);
					p0ColPoses.emplace_back(p0ColPos);
					p1ColPoses.emplace_back(p1ColPos);
				}
			}

			{
				glm::dvec3 planeNormal{};
				glm::dvec3 planePosition{};
				glm::dvec3 p0ColPos{};
				glm::dvec3 p1ColPos{};

				auto hasCollision = HasContiniousEdgeCollision(
					deltaTime,
					triangle.edgeVertices[2],
					triangle.edgeVertices[0],
					p0PrevPos,
					p0NextPos,
					p1PrevPos,
					p1NextPos,
					planeNormal,
					planePosition,
					p0ColPos,
					p1ColPos,
					debug
				);

				if (hasCollision == true)
				{
					planeNormals.emplace_back(planeNormal);
					planePositions.emplace_back(planePosition);
					p0ColPoses.emplace_back(p0ColPos);
					p1ColPoses.emplace_back(p1ColPos);
				}
			}
		}

		return planePositions.empty() == false;
	}


	//-------------------------------------------------------------------------------------------------

	bool HasContiniousEdgeCollision(
		double deltaTime,

		glm::dvec3 const& e0Pos,
		glm::dvec3 const& e1Pos,

		glm::dvec3 const& p0PrevPos_,
		glm::dvec3 const& p0NextPos_,
		glm::dvec3 const& p1PrevPos_,
		glm::dvec3 const& p1NextPos_,

		glm::dvec3& planeNormal,
		glm::dvec3& planePosition,
		glm::dvec3& p0ColPos,
		glm::dvec3& p1ColPos,

		bool debug
	)
	{
		// TODO: Re-implement this part considering situations where v1 == v0 and v1 == 0 || v0 == 0
		double const epsilon = 1e-5;

		auto p0PrevPos = p0PrevPos_;
		auto p0NextPos = p0NextPos_;

		auto p1PrevPos = p1PrevPos_;
		auto p1NextPos = p1NextPos_;

		auto const P0Vec = p0NextPos - p0PrevPos;
		auto P0Mag = glm::length(P0Vec);
		glm::dvec3 const P0Dir = P0Mag > 0 ? P0Vec / P0Mag : glm::dvec3{};

		 p0PrevPos -= epsilon * P0Dir;
		 p0NextPos += epsilon * P0Dir;

		auto const P1Vec = p1NextPos - p1PrevPos;
		auto P1Mag = glm::length(P1Vec);
		glm::dvec3 const P1Dir = P1Mag > 0 ? P1Vec / P1Mag : glm::dvec3{};

		p1PrevPos -= epsilon * P1Dir;
		p1NextPos += epsilon * P1Dir;

		P0Mag += 2 * epsilon;
		P1Mag += 2 * epsilon;

		auto const v0 = P0Mag / deltaTime;
		auto const v1 = P1Mag / deltaTime;

		auto const f = v0 * P0Dir;
		auto const d = v1 * P1Dir;

		auto const A = p1PrevPos - p0PrevPos;
		auto const B = d - f;
		auto const C = p0PrevPos - e0Pos;
		auto const D = e1Pos - e0Pos;

		auto const fXD = glm::cross(f, D);
		auto const CXD = glm::cross(C, D);

		auto const J = glm::dot(B, fXD);

		auto const H = glm::dot(A, CXD);
		auto const I = glm::dot(B, CXD) + glm::dot(A, fXD);

		if (debug == true)
		{
			MFA_LOG_INFO(
				"=============P0: %f %f %f, P1: %f %f %f, e0: %f %f %f, e1: %f %f %f, f: %f %f %f, d: %f %f %f"
				, p0PrevPos.x, p0PrevPos.y, p0PrevPos.z
				, p1PrevPos.x, p1PrevPos.y, p1PrevPos.z
				, e0Pos.x, e0Pos.y, e0Pos.z
				, e1Pos.x, e1Pos.y, e1Pos.z
				, f.x, f.y, f.z
				, d.x, d.y, d.z
			);
		}

		std::vector<double> times {};
		// Important almost use epsilon instead of checking for pure zero
		if (J < glm::epsilon<float>())
		{
			if (I == 0.0)
			{
				if (debug == true)
				{
					MFA_LOG_INFO("I is zero, A.FXD: %f, B.CXD %f, A: %f, %f %f, FXD: %f, %f, %f", glm::dot(A, fXD), glm::dot(B, CXD), A.x, A.y, A.z, fXD.x, fXD.y, fXD.z);
				}
				return false;
			}
			else
			{
				auto const t = -H / I;
				times.emplace_back(t);
			}
		}
		else
		{
			auto const denom = (I * I) - (4 * J * H);
			if (denom < 0.0)
			{
				MFA_LOG_INFO("Denom negative");
				return false;
				// times.emplace_back(0);
				// times.emplace_back(deltaTime);
			}
			else
			{
				auto const sqrt = std::sqrt(denom);

				auto const OneOver2J = 1.0 / (2 * J);

				times.emplace_back((-I + sqrt) * OneOver2J);
				if (sqrt != 0.0)
				{
					times.emplace_back((-I - sqrt) * OneOver2J);
				}
			}
		}

		// Newer times are more important
		for (int i = 0; i < times.size() - 1; ++i)
		{
			for (int j = i + 1; j < times.size(); ++j)
			{
				if (times[i] > times[j])
				{
					auto const temp = times[i];
					times[i] = times[j];
					times[j] = temp;
				}
			}
		}


		for (auto const t : times)
		{
			if (t < 0 || t > deltaTime)
			{
				if (debug == true)
				{
					MFA_LOG_INFO("Time is invalid: %f", t);
				}
				continue;
			}

			p0ColPos = p0PrevPos + f * t;
			p1ColPos = p1PrevPos + d * t;

			auto const dot = glm::dot(p1ColPos - p0ColPos, glm::cross(p0ColPos - e0Pos, e1Pos - e0Pos));

			if (debug == true)
			{
				MFA_LOG_INFO("Dot is %f, T is %f", dot, t);
			}
			// Not every dot is valid so we discard them if they are too large
			if (std::abs(dot) < epsilon)
			{
				if (DoSegmentsIntersect(p0ColPos, p1ColPos, e0Pos, e1Pos, debug))
				{
					// TODO: I need a proper planeNormal
					planeNormal = glm::cross(p1ColPos - p0ColPos, e1Pos - e0Pos);//-glm::normalize(f + d);
					/*if (glm::dot(planeNormal, p0PrevPos - p0ColPos) < 0.0)
					{
						planeNormal
					}*/
					p0ColPos -= P0Dir * epsilon;
					p1ColPos -= P1Dir * epsilon;
					
					planePosition = e0Pos;

					if (debug == true)
					{
						MFA_LOG_INFO("Success");
					}

					return true;
				}
			}
		}

		return false;
	}

	//-------------------------------------------------------------------------------------------------

	bool IsInsideTriangle(Triangle const& triangle, glm::dvec3 const& point)
	{
		for (int i = 0; i < 3; ++i)
		{
			auto const dot = glm::dot(triangle.edgeNormals[i], point - triangle.edgeVertices[i]);
			if (dot > 0)
			{
				return false;
			}
		}
		return true;
	}

	//-------------------------------------------------------------------------------------------------

	std::tuple<bool, bool, bool> InsideTriangleStatus(Triangle const& triangle, glm::dvec3 const& point)
	{
		return {
			glm::dot(triangle.edgeNormals[0], point - triangle.edgeVertices[0]) > 0.0,
			glm::dot(triangle.edgeNormals[1], point - triangle.edgeVertices[1]) > 0.0,
			glm::dot(triangle.edgeNormals[2], point - triangle.edgeVertices[2]) > 0.0
		};
	}

	//-------------------------------------------------------------------------------------------------

	void CalcDistanceToTriangle(
		Triangle const& triangle,
		glm::dvec3 const& point,
		double& outSqrDistance,
		glm::dvec3& outPlanePosition
	)
	{
		glm::dvec3 const vector = point - triangle.center;
		double const dot = glm::dot(vector, triangle.normal);

		outSqrDistance = std::abs(dot);

		outPlanePosition = point - dot * triangle.normal;

		auto [isOutsideEdge0, isOutsideEdge1, isOutsideEdge2] = InsideTriangleStatus(triangle, outPlanePosition);

		if (isOutsideEdge0 == false && isOutsideEdge1 == false && isOutsideEdge2 == false)
		{
			return;
		}

		int score = 0;
		if (isOutsideEdge0 == true)
		{
			score += 1;
		}
		if (isOutsideEdge1 == true)
		{
			score += 2;
		}
		if (isOutsideEdge2 == true)
		{
			score += 4;
		}

		bool isValid = true;
		switch (score)
		{
		case 1:
			isValid = CalcSqrDistanceToEdge(
				triangle.edgeVertices[0],
				triangle.edgeVertices[1],
				point,
				outSqrDistance
			);
			break;
		case 2:
			isValid = CalcSqrDistanceToEdge(
				triangle.edgeVertices[1],
				triangle.edgeVertices[2],
				point,
				outSqrDistance
			);
			break;
		case 3:
			outSqrDistance = glm::length2(triangle.edgeVertices[1] - point);
			break;
		case 4:
			isValid = CalcSqrDistanceToEdge(
				triangle.edgeVertices[2],
				triangle.edgeVertices[0],
				point,
				outSqrDistance
			);
			break;
		case 5:
			outSqrDistance = glm::length2(triangle.edgeVertices[0] - point);
			break;
		case 6:
			outSqrDistance = glm::length2(triangle.edgeVertices[2] - point);
			break;
		default:
			MFA_ASSERT(false);
		}

		MFA_ASSERT(isValid == true);
	}

	//-------------------------------------------------------------------------------------------------

	bool CalcDistanceToTriangleFast(
		Triangle const& triangle, 
		glm::dvec3 const& point, 
		double& outSqrDistance,
		glm::dvec3& outPlanePosition
	)
	{
		glm::dvec3 const vector = point - triangle.center;
		double const dot = glm::dot(vector, triangle.normal);

		outSqrDistance = std::abs(dot);

		outPlanePosition = point - dot * triangle.normal;

		auto [isOutsideEdge0, isOutsideEdge1, isOutsideEdge2] = InsideTriangleStatus(triangle, outPlanePosition);

		if (isOutsideEdge0 == false && isOutsideEdge1 == false && isOutsideEdge2 == false)
		{
			return true;
		}

		outSqrDistance = std::numeric_limits<double>::max();
		return false;
	}

	//-------------------------------------------------------------------------------------------------

	bool CalcSqrDistanceToEdge(
		glm::dvec3 const& edgeVertex0,
		glm::dvec3 const& edgeVertex1,
		glm::dvec3 const& point,
		double& outSqrDistance
	)
	{
		outSqrDistance = 0.0;

		auto const edgeVec = edgeVertex1 - edgeVertex0;
		auto const edgeLen = glm::length(edgeVec);
		if (edgeLen <= 0.0)
		{
			return false;
		}

		auto const edgeDir = edgeVec / edgeLen;
		auto const top = -glm::dot(point - edgeVertex0, edgeVec);
		auto const bottom = glm::dot(edgeDir, edgeVec);

		if (bottom == 0.0)
		{
			return false;
		}

		auto const t = std::clamp(top / bottom, 0.0, 1.0);

		auto const closestPoint = edgeVertex0 + t * edgeDir;

		outSqrDistance = glm::length2(closestPoint - point);

		return true;
	}

	//-------------------------------------------------------------------------------------------------

	Triangle GenerateCollisionTriangle(glm::dvec3 const& p0, glm::dvec3 const& p1, glm::dvec3 const& p2)
	{
		Triangle triangle{};
		UpdateCollisionTriangle(p0, p1, p2, triangle);
		return triangle;
	}

	//-------------------------------------------------------------------------------------------------

	void UpdateCollisionTriangle(
		glm::dvec3 const& p0, 
		glm::dvec3 const& p1, 
		glm::dvec3 const& p2,
		Triangle& outTriangle
	)
	{
		auto const v10 = glm::normalize(p1 - p0);
		auto const v21 = glm::normalize(p2 - p1);
		auto const v02 = glm::normalize(p0 - p2);

		outTriangle.center = (p0 + p1 + p2) / 3.0;
		outTriangle.normal = glm::normalize(glm::cross(v10, v21));

		glm::dvec3 normal1 = glm::normalize(glm::cross(v10, outTriangle.normal));
		glm::dvec3 normal2 = glm::normalize(glm::cross(v21, outTriangle.normal));
		glm::dvec3 normal3 = glm::normalize(glm::cross(v02, outTriangle.normal));
		
		outTriangle.edgeVertices[0] = p0 + normal1 * 1e-5;
		outTriangle.edgeVertices[1] = p1 + normal2 * 1e-5;
		outTriangle.edgeVertices[2] = p2 + normal3 * 1e-5;

		//glm::dvec3 normal1 = (glm::cross(v10, outTriangle.normal));
		//glm::dvec3 normal2 = (glm::cross(v21, outTriangle.normal));
		//glm::dvec3 normal3 = (glm::cross(v02, outTriangle.normal));

		outTriangle.edgeVertices[0] = p0;
		outTriangle.edgeVertices[1] = p1;
		outTriangle.edgeVertices[2] = p2;

		outTriangle.edgeNormals[0] = normal1;
		outTriangle.edgeNormals[1] = normal2;
		outTriangle.edgeNormals[2] = normal3;
	}

	//-------------------------------------------------------------------------------------------------

	StaticTriangleGrid::StaticTriangleGrid(
		std::vector<Triangle> triangles,
		glm::vec3 const& boundaryMin,
		glm::vec3 const& boundaryMax
	)
		: _triangles(std::move(triangles))
		, _boundaryMin(boundaryMin)
		, _boundaryMax(boundaryMax)
	{
		MFA_ASSERT(_triangles.empty() == false);

		double edgeLenSum = 0.0;
		int edgeCount = 0;
		for (auto const& triangle : _triangles)
		{
			edgeLenSum += glm::length(triangle.edgeVertices[1] - triangle.edgeVertices[0]);
			edgeLenSum += glm::length(triangle.edgeVertices[2] - triangle.edgeVertices[1]);
			edgeLenSum += glm::length(triangle.edgeVertices[0] - triangle.edgeVertices[2]);

			edgeCount += 3;
		}

		_cubeLength = static_cast<float>(edgeLenSum / static_cast<double>(edgeCount)) * 2.0f;

		Init();
	}

	//-------------------------------------------------------------------------------------------------

	StaticTriangleGrid::StaticTriangleGrid(
		std::vector<Triangle> triangles,
		glm::vec3 const& boundaryMin,
		glm::vec3 const& boundaryMax,
		float const cubeLength
	)
		: _triangles(std::move(triangles))
		, _boundaryMin(boundaryMin)
		, _boundaryMax(boundaryMax)
		, _cubeLength(cubeLength)
	{
		Init();
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Triangle*> const& StaticTriangleGrid::GetNearbyTriangles(glm::vec3 const& position) const
	{
		auto const [xIdx, yIdx, zIdx] = PositionToIdx(position);
		auto const cubeIdx = GetCubeIdx(xIdx, yIdx, zIdx);
		if (cubeIdx < 0)
		{
			return _emptyList;
		}
		return _cubes[cubeIdx].items;
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Triangle*> StaticTriangleGrid::GetNearbyTriangles(
		glm::vec3 const& position1,
		glm::vec3 const& position2
	) const
	{
		std::vector<Triangle*> triangles{};
		std::set<int> triangleSet{};

		auto [xs, ys, zs] = PositionToIdx(position1);
		auto [xe, ye, ze] = PositionToIdx(position2);

		int score = 0;
		if (xe == xs)
		{
			score += 1;
		}
		if (ye == ys)
		{
			score += 2;
		}
		if (ze == zs)
		{
			score += 4;
		}

		std::vector<int> cubeIndices{};

		switch (score)
		{
		case 0:
		{
			auto indices = Math::Rasterize(xs, ys, zs, xe, ye, ze);
			for (auto const& [x, y, z] : indices)
			{
				cubeIndices.emplace_back(GetCubeIdx(x, y, z));
			}
			break;
		}
		case 1:
		{
			auto indices = Math::Rasterize(ys, zs, ye, ze);
			for (auto const& [y, z] : indices)
			{
				cubeIndices.emplace_back(GetCubeIdx(xs, y, z));
			}
			break;
		}
		case 2:
		{
			auto indices = Math::Rasterize(xs, zs, xe, ze);
			for (auto const& [x, z] : indices)
			{
				cubeIndices.emplace_back(GetCubeIdx(x, ys, z));
			}
			break;
		}
		case 3:
		{
			int start = zs;
			int end = ze;
			if (start > end)
			{
				auto const temp = end;
				end = start;
				start = temp;
			}
			for (int z = start; z <= end; ++z)
			{
				cubeIndices.emplace_back(GetCubeIdx(xs, ys, z));
			}
			break;
		}
		case 4:
		{
			auto indices = Math::Rasterize(xs, ys, xe, ye);
			for (auto const& [x, y] : indices)
			{
				cubeIndices.emplace_back(GetCubeIdx(x, y, zs));
			}
			break;
		}
		case 5:
		{
			int start = ys;
			int end = ye;
			if (start > end)
			{
				auto const temp = end;
				end = start;
				start = temp;
			}
			for (int y = start; y <= end; ++y)
			{
				cubeIndices.emplace_back(GetCubeIdx(xs, y, zs));
			}
			break;
		}
		case 6:
		{
			int start = xs;
			int end = xe;
			if (start > end)
			{
				auto const temp = end;
				end = start;
				start = temp;
			}
			for (int x = start; x <= end; ++x)
			{
				cubeIndices.emplace_back(GetCubeIdx(x, ys, zs));
			}
			break;
		}
		case 7:
		{
			cubeIndices.emplace_back(GetCubeIdx(xs, ys, zs));
			break;
		}
		}

		for (auto const idx : cubeIndices)
		{
			for (auto& current : _cubes[idx].items)
			{
				if (triangleSet.contains(current->id) == false)
				{
					triangles.emplace_back(current);
					triangleSet.emplace(current->id);
				}
			}
		}

		return triangles;
	}

	//-------------------------------------------------------------------------------------------------

	std::vector<Triangle>& StaticTriangleGrid::GetTriangles()
	{
		return _triangles;
	}

	//-------------------------------------------------------------------------------------------------

	void StaticTriangleGrid::Init()
	{
		_boundaryLength = _boundaryMax - _boundaryMin;

		xGridCount = static_cast<int>(std::ceil(_boundaryLength.x / _cubeLength));
		yGridCount = static_cast<int>(std::ceil(_boundaryLength.y / _cubeLength));
		zGridCount = static_cast<int>(std::ceil(_boundaryLength.z / _cubeLength));

		_cubes.resize((xGridCount + 1) * (yGridCount + 1) * (zGridCount + 1));

		std::vector<std::future<std::vector<int>>> futures{};

		for (auto const& triangle : _triangles)
		{
			auto future = JS::Instance->AssignTask<std::vector<int>>([this, &triangle]()
				{
					std::vector<int> indices{};

					auto const [xIdx0, yIdx0, zIdx0] = PositionToIdx(triangle.edgeVertices[0]);
					auto const [xIdx1, yIdx1, zIdx1] = PositionToIdx(triangle.edgeVertices[1]);
					auto const [xIdx2, yIdx2, zIdx2] = PositionToIdx(triangle.edgeVertices[2]);

					auto const xMin = std::min(std::min(xIdx0, xIdx1), xIdx2);
					auto const yMin = std::min(std::min(yIdx0, yIdx1), yIdx2);
					auto const zMin = std::min(std::min(zIdx0, zIdx1), zIdx2);

					auto const xMax = std::max(std::max(xIdx0, xIdx1), xIdx2);
					auto const yMax = std::max(std::max(yIdx0, yIdx1), yIdx2);
					auto const zMax = std::max(std::max(zIdx0, zIdx1), zIdx2);

					for (int xIdx = xMin; xIdx <= xMax; ++xIdx)
					{
						for (int yIdx = yMin; yIdx <= yMax; ++yIdx)
						{
							for (int zIdx = zMin; zIdx <= zMax; ++zIdx)
							{
								auto const cubeIdx = GetCubeIdx(xIdx, yIdx, zIdx);
								indices.emplace_back(cubeIdx);
							}
						}
					}

					return indices;

					//std::set<int> indices{};
					//return std::vector<int>(indices.begin(), indices.end());
				});

			futures.emplace_back(std::move(future));
		}

		for (int i = 0; i < static_cast<int>(futures.size()); ++i)
		{
			auto const indices = futures[i].get();
			for (const auto cubeIdx : indices)
			{
				_cubes[cubeIdx].items.emplace_back(&_triangles[i]);
			}
		}
	}

	//-------------------------------------------------------------------------------------------------

	int StaticTriangleGrid::GetCubeIdx(int const xIdx, int const yIdx, int const zIdx) const
	{
		auto const idx = xIdx * yGridCount * zGridCount + yIdx * zGridCount + zIdx;

		if (idx >= _cubes.size())
		{
			return -1.0;
		}

		return idx;
	}

	//-------------------------------------------------------------------------------------------------

	std::tuple<int, int, int> StaticTriangleGrid::PositionToIdx(glm::vec3 const& position) const
	{
		int xIdx = static_cast<int>(std::floor((position.x - _boundaryMin.x) / _cubeLength));
		int yIdx = static_cast<int>(std::floor((position.y - _boundaryMin.y) / _cubeLength));
		int zIdx = static_cast<int>(std::floor((position.z - _boundaryMin.z) / _cubeLength));

		xIdx = std::clamp(xIdx, 0, xGridCount);
		yIdx = std::clamp(yIdx, 0, yGridCount);
		zIdx = std::clamp(zIdx, 0, zGridCount);

		return { xIdx, yIdx, zIdx };
	}

	//-------------------------------------------------------------------------------------------------

}
