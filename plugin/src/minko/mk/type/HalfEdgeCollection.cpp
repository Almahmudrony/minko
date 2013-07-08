#include "HalfEdgeCollection.hpp"

using namespace minko::parser::mk;

HalfEdgeCollection::HalfEdgeCollection(std::shared_ptr<minko::resource::IndexStream> indexStream)
{
	_indexStream = indexStream;

	initialize();
}

HalfEdgeCollection::~HalfEdgeCollection()
{
}

void
HalfEdgeCollection::initialize()
{
	unsigned int					id		= 0;
	std::vector<unsigned short>		data	= _indexStream->data();

	HalfEdgeMap map;

	for (unsigned int i = 0; i < data.size(); i += 3)
	{
		unsigned short t1 = data[i];
		unsigned short t2 = data[i + 1];
		unsigned short t3 = data[i + 2];

		HalfEdgePtr he1 = HalfEdge::create(t1, t2, id++);
		HalfEdgePtr he2 = HalfEdge::create(t2, t3, id++);
		HalfEdgePtr he3 = HalfEdge::create(t3, t1, id++);

		HalfEdgePtr halfEdges [3] = {he1, he2, he3};

		for (int edgeId = 0; edgeId < 3; ++edgeId)
		{
			halfEdges[edgeId]->setFace(he1, he2, he3);
			halfEdges[edgeId]->next(halfEdges[(edgeId + 1) % 3]);
			halfEdges[edgeId]->prec(halfEdges[(edgeId - 1) + 3 * (edgeId - 1 < 0 ? 1 : 0)]);
		}

		map[std::make_pair(t1, t2)] = he1;
		map[std::make_pair(t2, t3)] = he2;
		map[std::make_pair(t3, t1)] = he3;
		
		HalfEdgeMap::iterator adjacents [3] = {
			map.find(std::make_pair(t2, t1)),
			map.find(std::make_pair(t3, t2)),
			map.find(std::make_pair(t1, t3))};

		for (int edgeId = 0; edgeId < 3; ++edgeId)
		{
			if (adjacents[edgeId] == map.end())
				continue;

			halfEdges[edgeId]->adjacent(adjacents[edgeId]->second);
			adjacents[edgeId]->second->adjacent(halfEdges[edgeId]);
		}
	}

	for (HalfEdgeMap::iterator it = map.begin(); it != map.end(); it++)
	{
		std::cout << it->first.first << "  " << it->first.second << std::endl;
		std::cout << it->second << std::endl;
	}

	HalfEdgeMap unmarked(map.begin(), map.end());
	computeList(unmarked);
}

void
HalfEdgeCollection::computeList(HalfEdgeMap unmarked)
{
	std::queue<HalfEdgePtr> queue;

	while (unmarked.begin() != unmarked.end())
	{
		HalfEdgeList currentList;
		_subMeshesList.push_back(currentList);
		queue.push(unmarked.begin()->second);
		
		do
		{
			HalfEdgePtr he = queue.front();
			queue.pop();
			currentList.push_back(he);

			he->marked(true);		
			unmarked.erase(std::make_pair(he->startNodeId(), he->endNodeId()));

			if (he->adjacent() != nullptr && he->adjacent()->marked() == false)
			{
				HalfEdgeMap::iterator adjIt = unmarked.find(std::make_pair(he->adjacent()->startNodeId(), he->adjacent()->endNodeId()));
				unmarked.erase(std::make_pair(he->adjacent()->startNodeId(), he->adjacent()->endNodeId()));
				queue.push(he->adjacent());
			}
			if (he->next() != nullptr && he->next()->marked() == false)
			{
				HalfEdgeMap::iterator adjIt = unmarked.find(std::make_pair(he->next()->startNodeId(), he->next()->endNodeId()));
				unmarked.erase(std::make_pair(he->next()->startNodeId(), he->next()->endNodeId()));
				queue.push(he->next());
			}
			if (he->prec() != nullptr && he->prec()->marked() == false)
			{
				HalfEdgeMap::iterator adjIt = unmarked.find(std::make_pair(he->prec()->startNodeId(), he->prec()->endNodeId()));
				unmarked.erase(std::make_pair(he->prec()->startNodeId(), he->prec()->endNodeId()));
				queue.push(he->prec());
			}
		} while (queue.size() > 0);
	}

	std::cout << "Submeshes : " << _subMeshesList.size() << std::endl;
}