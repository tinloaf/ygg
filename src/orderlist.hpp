//
// Created by lukas on 09.08.17.
//

#ifndef YGG_ORDERLIST_HPP
#define YGG_ORDERLIST_HPP

namespace ygg {
namespace utilities {

template<class Node, bool enable>
class OrderListItem;

template<class Node>
class OrderListItem<Node, false>
{};

template<class Node>
class OrderListItem<Node, true>
{
public:
	static void insert_between(Node & n, Node * predecessor, Node * successor) {
		long long pred_point = std::numeric_limits<long long>::min();
		long long succ_point = std::numeric_limits<long long>::max();

		if (predecessor != nullptr) {
			pred_point = predecessor->_rbt_point;
		}

		if (successor != nullptr) {
			succ_point = successor->_rbt_point;
		}

		assert(pred_point < succ_point);

		if ((succ_point - pred_point) < 2) {
			if (successor != nullptr) {
				move_right(*successor);
				succ_point = successor->_rbt_point;
			}
			if (predecessor != nullptr) {
				move_left(*predecessor);
				pred_point = predecessor->_rbt_point;
			}
		}

		assert(pred_point < succ_point);
		assert(((pred_point < 0) && (succ_point > 0)) || (succ_point - pred_point) >= 2);

		insert_between_points(n, pred_point, succ_point);
	};

	// TODO make private?
	long long _rbt_point;
private:
	static void insert_between_points(Node & n, long long pred_point, long long succ_point)
	{
		long long pred_half = pred_point / 2;
		long long pred_remainder = pred_point % 2;
		long long succ_half = succ_point / 2;
		long long succ_remainder = succ_point % 2;

		long long new_point = pred_half + succ_half;
		if (pred_remainder && succ_remainder) {
			new_point += 1;
		}

		n._rbt_point = new_point;
	}

	static void move_right(Node & n)
	{
		Node * cur = &n;
		Node * stop = n._rbt_prev;

		while (cur->_rbt_next != nullptr) {
			cur = cur->_rbt_next;
		}

		while (cur != stop) {
			insert_between_points(*cur, cur->_rbt_point, cur->_rbt_next != nullptr ?
			                                             cur->_rbt_next->_rbt_point :
			                                             std::numeric_limits<long long>::max());
			cur = cur->_rbt_prev;
		}
	}

	static void move_left(Node & n)
	{
		Node * cur = &n;
		Node * stop = n._rbt_next;

		while (cur->_rbt_prev != nullptr) {
			cur = cur->_rbt_prev;
		}

		while (cur != stop) {
			insert_between_points(*cur, cur->_rbt_point, cur->_rbt_prev != nullptr ?
			                                             cur->_rbt_prev->_rbt_point :
			                                             std::numeric_limits<long long>::min());
			cur = cur->_rbt_next;
		}
	}
};

} // namespace utilities
} // namespace ygg

#endif //YGG_ORDERLIST_HPP
