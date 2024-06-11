package navi

import (
	"container/heap"
	"encoding/json"
	"fmt"
	config "gowebserver/WebServer/common"
	models "gowebserver/WebServer/model"
	"io"
	"math"
	"math/rand"
	"os"
	"sort"
	"strconv"
)

func (pq PriorityQueue) Len() int           { return len(pq) }
func (pq PriorityQueue) Less(i, j int) bool { return pq[i].priority < pq[j].priority }
func (pq PriorityQueue) Swap(i, j int)      { pq[i], pq[j] = pq[j], pq[i]; pq[i].index = i; pq[j].index = j }
func (pq *PriorityQueue) Push(x interface{}) {
	n := len(*pq)
	item := x.(*Item)
	item.index = n
	*pq = append(*pq, item)
}
func (pq *PriorityQueue) Pop() interface{} {
	old := *pq
	n := len(old)
	item := old[n-1]
	old[n-1] = nil
	item.index = -1
	*pq = old[0 : n-1]
	return item
}

// calculateTotalDistance calculates the total distance for a given path in the graph
func calculateTotalDistance(graph *Graph, path []int) int {
	totalDistance := 0
	if len(path) < 2 {
		return 0 // If the path has less than two points, the distance is zero
	}

	// Iterate through the path array, summing the distances between consecutive points
	for i := 0; i < len(path)-1; i++ {
		current := path[i]
		next := path[i+1]

		// Find the edge between the current and next node and add its weight to the total distance
		for _, edge := range graph.AdjList[fmt.Sprintf("%d", current)] {
			if edge.Destination == next {
				totalDistance += edge.Weight
				break
			}
		}
	}
	return totalDistance
}
func dijkstraWithWaypoints(graph *Graph, waypoints []int, objectType string) ([]int, *Point) {
	fullPath := []int{}
	closestObject := &Point{Index: -1, Type: "None"}
	closestObjectDistance := math.MaxInt64

	for i := 0; i < len(waypoints)-1; i++ {
		partialPath, tempClosestObject, tempClosestObjectDistance := dijkstraPath(graph, waypoints[i], waypoints[i+1], objectType)
		fullPath = append(fullPath, partialPath[:len(partialPath)-1]...) // Append partial path except the last node to avoid duplication

		// Check and update closest object information
		if tempClosestObjectDistance < closestObjectDistance {
			closestObject = tempClosestObject
			closestObjectDistance = tempClosestObjectDistance
		}
	}
	fullPath = append(fullPath, waypoints[len(waypoints)-1]) // Append the last target point

	return fullPath, closestObject
}

// dijkstraPath finds the shortest path between two points and the closest object of a given type using Dijkstra's algorithm

func dijkstraPath(graph *Graph, start, target int, objectType string) ([]int, *Point, int) {
	dist := make(map[int]int)
	prev := make(map[int]int)
	pq := make(PriorityQueue, 0)
	heap.Init(&pq)
	closestObject := &Point{Index: -1, Type: "None"}
	closestObjectDistance := math.MaxInt64

	for _, p := range graph.Points {
		dist[p.Index] = math.MaxInt64
		prev[p.Index] = -1
		heap.Push(&pq, &Item{vertex: p.Index, priority: math.MaxInt64})
	}

	dist[start] = 0
	heap.Push(&pq, &Item{vertex: start, priority: 0})

	for pq.Len() > 0 {
		u := heap.Pop(&pq).(*Item).vertex
		if u == target {
			break
		}

		for _, edge := range graph.AdjList[fmt.Sprintf("%d", u)] {
			alt := dist[u] + edge.Weight
			if alt < dist[edge.Destination] {
				dist[edge.Destination] = alt
				prev[edge.Destination] = u
				heap.Push(&pq, &Item{vertex: edge.Destination, priority: alt})
			}
		}

		// Update the closest object information if this point is of the specified type
		if graph.Points[u].Type == objectType && dist[u] < closestObjectDistance {
			closestObject = &graph.Points[u]
			closestObjectDistance = dist[u]
		}
	}

	// Build path
	// Build path
	path := []int{}
	step := target
	for step != -1 {
		path = append([]int{step}, path...) // 将当前 step 添加到 path 的开始
		step = prev[step]                   // 更新 step 为前一个节点
	}
	return path, closestObject, closestObjectDistance
}

// 提取并排序终点相连的景点
func extractAndSortScenic(graph *Graph, target int, maxDistance float64) []Point {
	scenics := []Point{}

	targetPoint := graph.Points[target]
	// 检查图中所有的景点
	for _, point := range graph.Points {
		if point.Type == "Scenic" && distance(targetPoint, point) <= maxDistance {
			scenics = append(scenics, point)
		}
	}

	// 根据评分排序
	sort.Slice(scenics, func(i, j int) bool {
		return scenics[i].Rating > scenics[j].Rating // 降序
	})

	return scenics
}

// 计算两点之间的欧几里得距离
func distance(p1, p2 Point) float64 {
	return math.Sqrt(float64((p1.X-p2.X)*(p1.X-p2.X) + (p1.Y-p2.Y)*(p1.Y-p2.Y)))
}

func dijkstraWithTime(graph *Graph, waypoints []int) []int {

	fullPath := []int{}

	for i := 0; i < len(waypoints)-1; i++ {
		partialPath := dijkstraPathTime(graph, waypoints[i], waypoints[i+1])
		fullPath = append(fullPath, partialPath[:len(partialPath)-1]...) // Append partial path except the last node to avoid duplication
	}
	fullPath = append(fullPath, waypoints[len(waypoints)-1]) // Append the last target point

	return fullPath
}

// dijkstraPath finds the shortest path between two points and the closest object of a given type using Dijkstra's algorithm

func dijkstraPathTime(graph *Graph, start int, target int) []int {
	dist := make(map[int]int)
	prev := make(map[int]int)
	pq := make(PriorityQueue, 0)
	heap.Init(&pq)

	for _, p := range graph.Points {
		dist[p.Index] = math.MaxInt64
		prev[p.Index] = -1
		heap.Push(&pq, &Item{vertex: p.Index, priority: math.MaxInt64})
	}

	dist[start] = 0
	heap.Push(&pq, &Item{vertex: start, priority: 0})

	for pq.Len() > 0 {
		u := heap.Pop(&pq).(*Item).vertex
		if u == target {
			break
		}

		for _, edge := range graph.AdjList[fmt.Sprintf("%d", u)] {
			alt := dist[u] + edge.Time
			if alt < dist[edge.Destination] {
				dist[edge.Destination] = alt
				prev[edge.Destination] = u
				heap.Push(&pq, &Item{vertex: edge.Destination, priority: alt})
			}
		}
	}

	// Build path
	// Build path
	path := []int{}
	step := target
	for step != -1 {
		path = append([]int{step}, path...) // 将当前 step 添加到 path 的开始
		step = prev[step]                   // 更新 step 为前一个节点
	}
	return path
}

func reconstructPath(prev map[int]int, target int) []int {
	var path []int
	for step := target; step != -1; step = prev[step] {
		path = append([]int{step}, path...)
	}
	return path
}

func calculateTotalTime(graph *Graph, path []int) int {
	totalTime := 0
	if len(path) < 2 {
		return 0
	}
	for i := 0; i < len(path)-1; i++ {
		current := path[i]
		next := path[i+1]
		for _, edge := range graph.AdjList[fmt.Sprintf("%d", current)] {
			if edge.Destination == next {
				totalTime += edge.Time
				break
			}
		}
	}
	return totalTime
}

func calculatePaths(graph Graph, start int, target int, temp []int) PathResults {
	waypoints := append([]int{start}, temp...)
	waypoints = append(waypoints, target)

	path, closestObToi := dijkstraWithWaypoints(&graph, waypoints, "Toilet")
	path, closestObRes := dijkstraWithWaypoints(&graph, waypoints, "Restaurant")
	totalDistance := calculateTotalDistance(&graph, path)
	timePath := dijkstraWithTime(&graph, waypoints)
	totalTime := calculateTotalTime(&graph, timePath)

	return PathResults{
		ClosestRes:    closestObRes,
		ClosestToi:    closestObToi,
		Path:          path,
		TotalDistance: totalDistance,
		TimePath:      timePath,
		TotalTime:     totalTime,
	}
}

func Navi(req models.NaviReq) (results PathResults) {
	// 读取并解析JSON文件
	graphId := req.GraphId
	start := req.Start
	target := req.Target
	temp := req.Temp
	fileName := config.MapPath + "graph-" + strconv.Itoa(graphId) + ".json"
	jsonFile, err := os.Open(fileName)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer jsonFile.Close()

	byteValue, _ := io.ReadAll(jsonFile)

	var graph Graph
	if err := json.Unmarshal(byteValue, &graph); err != nil {
		fmt.Println("Error parsing JSON:", err)
		return
	}
	pointsJSON, _ := json.MarshalIndent(graph.Points, "", "  ")
	mapName := config.MapPath + "map-" + strconv.Itoa(graphId) + ".json"
	os.WriteFile(mapName, pointsJSON, 0644)

	// 为每个边计算 Time
	for key := range graph.AdjList {
		for i := range graph.AdjList[key] {
			graph.AdjList[key][i].Time = int(100 * float64(graph.AdjList[key][i].Weight) * (rand.Float64()))
		}
	}

	results = calculatePaths(graph, start, target, temp)

	// 将所有结果保存到一个 JSON 文件中
	resultsJSON, err := json.MarshalIndent(results, "", "  ")
	if err != nil {
		fmt.Println("Error marshalling results to JSON:", err)
		return
	}
	resultName := config.MapPath + "result-" + strconv.Itoa(graphId) + ".json"
	os.WriteFile(resultName, resultsJSON, 0644)
	fmt.Println("All path results saved to " + resultName)

	return
}

func Scan(req models.ScanReq) (scenics []Point) {
	graphId := req.GraphId
	target := req.PresentIdx
	dist := req.Dist
	fileName := config.MapPath + "graph-" + strconv.Itoa(graphId) + ".json"
	jsonFile, err := os.Open(fileName)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer jsonFile.Close()

	byteValue, _ := io.ReadAll(jsonFile)

	var graph Graph
	if err := json.Unmarshal(byteValue, &graph); err != nil {
		fmt.Println("Error parsing JSON:", err)
		return
	}
	pointsJSON, _ := json.MarshalIndent(graph.Points, "", "  ")
	mapName := config.MapPath + "map-" + strconv.Itoa(graphId) + ".json"
	os.WriteFile(mapName, pointsJSON, 0644)
	// 提取并排序终点相连的景点
	scenics = extractAndSortScenic(&graph, target, float64(dist))

	scenicJSON, _ := json.MarshalIndent(scenics, "", "  ")
	scenicName := config.MapPath + "scenic-" + strconv.Itoa(graphId) + ".json"
	os.WriteFile(scenicName, scenicJSON, 0644)
	fmt.Println("Scenic points connected to", target, "sorted by rating:")
	for _, scenic := range scenics {
		fmt.Printf("Name: %s, Rating: %.1f\n", scenic.Name, scenic.Rating)
	}

	return
}
