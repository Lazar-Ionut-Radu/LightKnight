import re
import subprocess
from dataclasses import dataclass

def play_match(
    engine_path: str,
    params_path1: str,
    params_path2: str,
    games: int,
    time_control: str = "5+0.05",
    concurrency: int = 8,
):
    command = [
        "fastchess",
        
        "-engine",
        f"cmd={engine_path}",
        "name=LightKnight_A",
        f"option.ParametersFile={params_path1}",
        f"option.LoadParameters=true",
        
        "-engine",
        f"cmd={engine_path}",
        "name=LightKnight_B",
        f"option.ParametersFile={params_path2}",
        f"option.LoadParameters=true",
        
        "-each",
        f"tc={time_control}",
        
        "-openings",
        "file=./tools/tuner/8moves_v3.pgn",
        "format=pgn",
        "order=random",
        
        "-rounds",
        str(games),
        "-repeat",
        
        "-concurrency",
        str(concurrency),
    ]
    
    result = subprocess.run(
        command,
        capture_output=True,
        text=True,
    )
    
    return result.stdout
        
def parse_results(output: str) -> list[dict]:
    results = []

    blocks = output.split("--------------------------------------------------")

    for block in blocks:
        elo_match = re.search(
            r"Elo:\s*([+-]?\d+(?:\.\d+)?)\s*\+/-\s*([+-]?\d+(?:\.\d+)?)",
            block,
        )

        nelo_match = re.search(
            r"nElo:\s*([+-]?\d+(?:\.\d+)?)\s*\+/-\s*([+-]?\d+(?:\.\d+)?)",
            block,
        )

        games_match = re.search(
            r"Games:\s*(\d+),\s*Wins:\s*(\d+),\s*Losses:\s*(\d+),\s*Draws:\s*(\d+)",
            block,
        )

        # Only consider this a result block if we found the main result data.
        if not elo_match or not nelo_match or not games_match:
            continue

        result = {
            "elo": float(elo_match.group(1)),
            "elo_err": float(elo_match.group(2)),
            "nelo": float(nelo_match.group(1)),
            "nelo_err": float(nelo_match.group(2)),

            "games": int(games_match.group(1)),
            "wins": int(games_match.group(2)),
            "losses": int(games_match.group(3)),
            "draws": int(games_match.group(4)),
        }

        results.append(result)

    return results

if __name__ == "__main__":
    out = play_match(
        "./build/release/lightknight",
        "./params.csv",
        "./params.csv",
        40,
        "1+0.01",
        8
    )
    
    results = parse_results(out)
    print (results[-1])