#pragma once

#include <vec3.hpp>
#include <vector>
#include <set>

namespace MFA::Collision
{
    class StaticTriangleGrid;

    struct Triangle
    {
        inline static int nextId = 0;
        explicit Triangle()
        {
            id = ++nextId;
        }

        int id = 0;
        glm::dvec3 center{};
        glm::dvec3 normal{};
        glm::dvec3 edgeNormals[3]{};
        glm::dvec3 edgeVertices[3]{};
        bool selected = false;
    };

    // This function can only handle external collision
    [[nodiscard]]
    bool HasIntersection(
        Triangle const& triangle,
        glm::dvec3 const& currentPos,
        glm::dvec3 const& previousPos,
        glm::dvec3& outCollisionPos,
        double& outTime,
        double epsilon,
        bool checkForBackCollision
    );

    [[nodiscard]]
    bool HasIntersection(
        Triangle const& triangle,
        glm::dvec3 const& currentPos,
        glm::dvec3 const& previousPos,
        glm::dvec3& outCollisionPos,
        double epsilon,
        bool checkForBackCollision
    );

    bool HasDynamicIntersection(
        glm::dvec3 const& pointPrevPos,
        glm::dvec3 const& pointCurrPos,

        glm::dvec3 const& triPrevPos0,
        glm::dvec3 const& triPrevPos1,
        glm::dvec3 const& triPrevPos2,

        glm::dvec3 const& triCurrPos0,
        glm::dvec3 const& triCurrPos1,
        glm::dvec3 const& triCurrPos2,

        double epsilon,
        bool checkForBackCollision
    );

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
        double& collisionTime,
        bool checkForSegmentIntersection = true
    );

    [[nodiscard]]
	bool IsInside(std::vector<Triangle> const& triangles, glm::dvec3 const& point);

    [[nodiscard]]
	bool IsInside(StaticTriangleGrid& grid, glm::dvec3 const & point);
    // TODO: I can unify this with a callback
    [[nodiscard]]
	bool FindClosestTriangle(
		std::vector<Triangle> const& triangles,
		glm::dvec3 const& point,
		int& outTriangleIdx,
		glm::dvec3& outTrianglePosition,
		glm::dvec3& outTriangleNormal
	);

    [[nodiscard]]
    bool FindClosestTriangle(
        StaticTriangleGrid& grid,
        glm::dvec3 const& point,
        int& outTriangleIdx,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal
    );

    [[nodiscard]]
    bool FindClosestTriangle(
        std::vector<Triangle const *> const& triangles,
        glm::dvec3 const& point,
        int& outTriangleIdx,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal
    );

    // Continuous collision

    [[nodiscard]]
    bool HasContiniousCollision(
        std::vector<Triangle>& triangles,
        glm::dvec3 const& prevPos,
        glm::dvec3 const& nextPos,
        int& outTriangleIdx,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal
    );

    [[nodiscard]]
    bool HasContiniousCollision(
        StaticTriangleGrid& grid,
        glm::dvec3 const& prevPos,
        glm::dvec3 const& nextPos,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal
    );

    [[nodiscard]]
    bool HasContiniousCollision(
        std::vector<Triangle const*>& triangles,
        glm::dvec3 const& prevPos,
        glm::dvec3 const& nextPos,
        int& outTriangleIdx,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal,
        double epsilon = 0.0//1e-1
    );

    // Static collision

    // This function can only handle external collision
    [[nodiscard]]
    bool HasStaticCollision(
	    const std::vector<Triangle>& triangles,
        glm::dvec3 const& point,
        int& outTriangleIdx,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal
    );

    [[nodiscard]]
    bool HasStaticCollision(
        StaticTriangleGrid& grid,
        glm::dvec3 const& point,
        glm::dvec3& outTrianglePosition,
        glm::dvec3& outTriangleNormal
    );

    [[nodiscard]]
    bool HasSelfCollision(
        const std::vector<Triangle>& triangles,
        glm::dvec3 const& point,
        double thickness,
        int& outTriangleIdx
    );

    [[nodiscard]]
    std::vector<Triangle const *> HasDiscreteEdgeCollision(
        StaticTriangleGrid const& grid,
        glm::dvec3 const& myEdge0,
        glm::dvec3 const& myEdge1
    );

    [[nodiscard]]
    std::vector<Triangle const *> HasDiscreteEdgeCollision(
        const std::vector<Triangle>& triangles,
        glm::dvec3 const& myEdge0,
        glm::dvec3 const& myEdge1
    );

    [[nodiscard]]
    bool DoSegmentsIntersect(
        glm::dvec3 const& p0,
        glm::dvec3 const& p1,
        glm::dvec3 const& q0,
        glm::dvec3 const& q1,

        bool debug = false
    );

    [[nodiscard]]
    bool HasContiniousEdgeCollision(
        double deltaTime,

        StaticTriangleGrid const& grid,

        glm::dvec3 const& p0PrevPos,
        glm::dvec3 const& p0NextPos,
        glm::dvec3 const& p1PrevPos,
        glm::dvec3 const& p1NextPos,

        std::vector<glm::dvec3> & planeNormal,
        std::vector<glm::dvec3> & planePosition,
        std::vector<glm::dvec3> & p0ColPos,
        std::vector<glm::dvec3> & p1ColPos,

        bool debug = false
    );

    [[nodiscard]]
    bool HasContiniousEdgeCollision(
        double deltaTime,

        const std::vector<Triangle>& triangles,

        glm::dvec3 const& p0PrevPos,
        glm::dvec3 const& p0NextPos,
        glm::dvec3 const& p1PrevPos,
        glm::dvec3 const& p1NextPos,

        std::vector<glm::dvec3> & planeNormals,
        std::vector<glm::dvec3> & planePositions,
        std::vector<glm::dvec3> & p0ColPoses,
        std::vector<glm::dvec3> & p1ColPoses,

        bool debug = false
    );

    bool HasContiniousEdgeCollision(
        double deltaTime,

        glm::dvec3 const& e0Pos,
        glm::dvec3 const& e1Pos,

        glm::dvec3 const& p0PrevPos,
        glm::dvec3 const& p0NextPos,
        glm::dvec3 const& p1PrevPos,
        glm::dvec3 const& p1NextPos,

        glm::dvec3 & planeNormal,
        glm::dvec3 & planePosition,
        glm::dvec3 & p0ColPos,
        glm::dvec3 & p1ColPos,

        bool debug = false
    );

    // Passed point must be on the triangle plane
    [[nodiscard]]
    bool IsInsideTriangle(Triangle const& triangle, glm::dvec3 const& point);

    /*
        Passed point must be on the triangle plane
        Returns true if point is outside
    */
    [[nodiscard]]
    std::tuple<bool, bool, bool> InsideTriangleStatus(Triangle const& triangle, glm::dvec3 const& point);

    void CalcDistanceToTriangle(
        Triangle const& triangle, 
        glm::dvec3 const& point, 
        double & outSqrDistance, 
        glm::dvec3& outPlanePosition
    );

    bool CalcDistanceToTriangleFast(
        Triangle const& triangle,
        glm::dvec3 const& point,
        double& outSqrDistance,
        glm::dvec3& outPlanePosition
    );

    [[nodiscard]]
    bool CalcSqrDistanceToEdge(
        glm::dvec3 const& edgeVertex0,
        glm::dvec3 const& edgeVertex1,
        glm::dvec3 const& point,
        double& outSqrDistance
    );

    [[nodiscard]]
    Triangle GenerateCollisionTriangle(
        glm::dvec3 const& p0,
        glm::dvec3 const& p1,
        glm::dvec3 const& p2
    );

    void UpdateCollisionTriangle(
        glm::dvec3 const& p0,
        glm::dvec3 const& p1,
        glm::dvec3 const& p2,
        Triangle& outTriangle
    );

    // Idea for efficient grid
    // https://stackoverflow.com/questions/9047612/glmivec2-as-key-in-unordered-map
    // Optimal size of a grid is the edge length average based on the spatial hashing paper
    template<typename T>
    struct GridCube
    {
        std::vector<T> items{};
        int version{};
    };

    class StaticTriangleGrid
    {
    public:

        using Cube = GridCube<Triangle*>;

        explicit StaticTriangleGrid() = default;

        // Computes cube length by computing the average edge length
        explicit StaticTriangleGrid(
            std::vector<Triangle> triangles,
            glm::vec3 const& boundaryMin,
            glm::vec3 const& boundaryMax
        );

        explicit StaticTriangleGrid(
            std::vector<Triangle> triangles,
            glm::vec3 const& boundaryMin,
            glm::vec3 const& boundaryMax,
            float cubeLength
        );

        [[nodiscard]]
        std::vector<Triangle*> const& GetNearbyTriangles(glm::vec3 const& position) const;

        // TODO: I can return Triangle& instead
        [[nodiscard]]
        std::vector<Triangle*> GetNearbyTriangles(glm::vec3 const& position1, glm::vec3 const& position2) const;

        [[nodiscard]]
        std::vector<Triangle>& GetTriangles();

    private:

        void Init();

        [[nodiscard]]
        int GetCubeIdx(int xIdx, int yIdx, int zIdx) const;

        [[nodiscard]]
        std::tuple<int, int, int> PositionToIdx(glm::vec3 const& position) const;

        std::vector<Triangle> _triangles{};
        std::vector<Triangle*> _emptyList{};

        glm::vec3 _boundaryMin{};
        glm::vec3 _boundaryMax{};
        glm::vec3 _boundaryLength{};
        float _cubeLength{};

        std::vector<Cube> _cubes{};

        int xGridCount{};
        int yGridCount{};
        int zGridCount{};
    };
}

namespace MFA
{
    using CollisionTriangle = Collision::Triangle;
    using StaticCollisionGrid = Collision::StaticTriangleGrid;
}
