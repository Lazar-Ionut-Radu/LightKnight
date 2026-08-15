import argparse
import csv
import chess
import chess.pgn


def result_to_score(result: str) -> float:
    if result == "1-0":
        return 1.0
    if result == "1/2-1/2":
        return 0.5
    if result == "0-1":
        return 0.0

    raise ValueError(f"Unknown game result: {result}")


def pgn_to_positions(
    pgn_path: str,
    output_path: str,
    sample_interval: int = 1,
    skip_book_moves: bool = True
):
    positions = {}

    with open(pgn_path, encoding="utf-8") as pgn_file:
        games = 0
        total_positions = 0

        while game := chess.pgn.read_game(pgn_file):
            games += 1

            result = result_to_score(game.headers["Result"])
            board = game.board()
            book_ended = False
            non_book_plies = 0

            node = game

            while node.variations:
                next_node = node.variations[0]

                board.push(next_node.move)

                if skip_book_moves and not book_ended:
                    if next_node.comment.strip().lower() == "book":
                        node = next_node
                        continue

                    book_ended = True

                non_book_plies += 1

                if non_book_plies % sample_interval == 0:
                    total_positions += 1
                    fen = board.fen()

                    # Convert result to side-to-move perspective.
                    score = result
                    if board.turn == chess.BLACK:
                        score = 1.0 - score

                    if fen not in positions:
                        positions[fen] = {
                            "wins": 0,
                            "draws": 0,
                            "losses": 0,
                        }

                    if score == 1.0:
                        positions[fen]["wins"] += 1
                    elif score == 0.5:
                        positions[fen]["draws"] += 1
                    else:
                        positions[fen]["losses"] += 1

                node = next_node

    with open(output_path, "w", newline="", encoding="utf-8") as output_file:
        writer = csv.writer(output_file)

        writer.writerow(["fen", "wins", "draws", "losses"])

        for fen, result in positions.items():
            writer.writerow([
                fen,
                result["wins"],
                result["draws"],
                result["losses"],
            ])

    print(f"Games parsed: {games}")
    print(f"Total positions: {total_positions}")
    print(f"Unique positions: {len(positions)}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Convert PGN games into a Texel tuning position dataset."
    )

    parser.add_argument("pgn_path")
    parser.add_argument("output_path")

    parser.add_argument(
        "--sample-interval",
        type=int,
        default=1,
        help="Sample every N non-book plies (default: 1)."
    )

    parser.add_argument(
        "--skip-book-moves",
        action=argparse.BooleanOptionalAction,
        default=True,
        help="Skip positions marked as book (default: enabled)."
    )

    args = parser.parse_args()

    pgn_to_positions(
        args.pgn_path,
        args.output_path,
        sample_interval=args.sample_interval,
        skip_book_moves=args.skip_book_moves,
    )