<?php

/* add line below to cron:

* * * * * root php /path/to/movies/cams.php > /dev/null 2>&1

*/

class MinimalFtp
{
	private $connection;

	public function isDir($directory)
	{
		$pwd = ftp_pwd($this->connection);

		if ($pwd === false) {
			return false;
		}

		$isDir = @ftp_chdir($this->connection, $directory);
		ftp_chdir($this->connection, $pwd);

		return $isDir;
	}

	public function mkdir($directory)
	{
		$pwd = ftp_pwd($this->connection);
		$parts = explode('/', trim($directory, '/'));

		foreach ($parts as $part) {
			if (!@ftp_chdir($this->connection, $part)) {
				if (!ftp_mkdir($this->connection, $part)) {
					ftp_chdir($this->connection, $pwd);

					return false;
				}

				ftp_chdir($this->connection, $part);
			}
		}

		ftp_chdir($this->connection, $pwd);

		return true;
	}

	public function send(string $host, string $username, string $password, string $directory, string $filename, string $hash)
	{
		$this->connection = ftp_connect($host);

		if (!$this->connection) {
			return false;
		}

		if (!ftp_login($this->connection, $username, $password)) {
			return false;
		}

		if (!ftp_pasv($this->connection, true)) {
			return false;
		}

		if (!$this->mkdir($directory)) {
			return false;
		}

		if (!ftp_put(
			$this->connection,
			$directory . '/' . $filename . '_' . substr($hash, 0, 16),
			__DIR__ . '/' . $filename
		)) {
			return false;
		}

		return ftp_close($this->connection);
	}
}

class Cam
{
	const LINE_ENDING = "<hr/>\n";
	private $mysqlConnection;

	public function execute($config)
	{
		$start = microtime(true);
		$cams = [];
		$this->mysqlConnection = new mysqli(
			$config['sqlHost'],
			$config['sqlUser'],
			$config['sqlPassword'],
			$config['sqlDatabase']
		);

		if ($this->mysqlConnection->connect_error) {
			die('MySQL Error: ' . $this->mysqlConnection->connect_error);
		}

		$files = scandir(__DIR__);
		$count = 0;
		$max = 50;

		foreach ($files as $file) {
			if (!strpos($file, '_')) {
				continue;
			}

			if (time() - filemtime(__DIR__ . '/' . $file) < 20) {
				continue;
			}

			$cams[] = $file;

			if (++$count >= $max) {
				break;
			}
		}

		echo 'cams: ' . count($cams) . self::LINE_ENDING;

		foreach ($cams as $cam) {
			@ob_flush();
			echo 'File ' . $cam . ': ';
			$movie_size = filesize(__DIR__ . '/' . $cam);

			if ($movie_size == 0) {
				unlink(__DIR__ . '/' . $cam);
				echo 'empty file' . self::LINE_ENDING;
				continue;
			}

			$data = $this->get_data(__DIR__ . '/' . $cam);
			$sum = $this->get_video_length($data);

			if ($sum <= 0) {
				unlink(__DIR__ . '/' . $cam);
				echo '0 seconds' . self::LINE_ENDING;
				continue;
			}

			$hex_sum = substr('00000000' . dechex($sum), -8);
			$data .= chr(hexdec('63')) . chr(hexdec($hex_sum[6] . $hex_sum[7])) . chr(hexdec($hex_sum[4] . $hex_sum[5]))
				. chr(hexdec($hex_sum[2] . $hex_sum[3])) . chr(hexdec($hex_sum[0] . $hex_sum[1]));
			list($player_id, $timestamp) = explode('_', $cam);
			$hash = md5($player_id . $timestamp);
			$directory = date('Ym', $timestamp / 1000);
			$this->compress($data, __DIR__ . '/' . $cam);

			if (!(new MinimalFtp())->send(
				$config['ftpHost'],
				$config['ftpUser'],
				$config['ftpPassword'],
				$directory . '/' . $player_id,
				$cam,
				$hash
			)) {
				echo 'ftp put failed' . self::LINE_ENDING;
				continue;
			}

			$player_data = $this->select('SELECT id AS player_id, account_id, name FROM players WHERE id = ' . $player_id);

			if (empty($player_data)) {
				$player_data[0] = [
					'account_id' => '1',
					'player_id' => $player_id,
					'name' => 'Unknown',
				];
			}

			$player_data = $player_data[0];
			$params = [
				'account_id' => $player_data['account_id'],
				'player_id' => $player_data['player_id'],
				'player_name' => $player_data['name'],
				'hash' => $hash,
				'directory' => $directory,
				'filename' => $cam,
				'duration' => $sum,
				'started' => date('Y-m-d H:i:s', substr($timestamp, 0, 10)),
				'ended' => date('Y-m-d H:i:s', substr($timestamp + $sum, 0, 10)),
				'parsed' => date('Y-m-d H:i:s'),
			];

			$insertQuery = 'INSERT IGNORE INTO cams (' . implode(', ', array_keys($params)) . ') VALUES ("' . implode('", "', array_values($params)) . '")';
			$this->select($insertQuery);
			$checkQuery = 'SELECT * FROM cams WHERE hash = "' . $hash . '"';

			if (!$this->select($checkQuery)) {
				echo 'failed to add cam data, query: ' . $insertQuery . self::LINE_ENDING;
				continue;
			}

			if (is_file(__DIR__ . '/' . $cam)) {
				unlink(__DIR__ . '/' . $cam);
			}

			echo 'OK' . self::LINE_ENDING;
		}

		echo 'cams: ' . count($cams) . ', time: ' . round(microtime(true) - $start, 3) . self::LINE_ENDING;
	}

	private function compress($data, $filename)
	{
		$fp = fopen($filename, 'w');
		fwrite($fp, gzencode($data, 7));
		fclose($fp);
	}

	private function get_data($filename)
	{
		$data = implode('', file($filename));
		$firstHex = substr('0' . dechex(ord($data[0])), -2);

		if ($firstHex == '1f') {
			return gzdecode($data);
		}

		return $data;
	}

	private function get_video_length($data)
	{
		$movie_size = strlen($data);
		$end = false;
		$offset = 0;
		$sum = 0;

		while (!$end) {
			$hex = substr('0' . dechex(ord($data[$offset])), -2);
			if ($hex == '63') {
				$offset += 5;
			} elseif ($hex == '64') {
				$offset += 3;
			} elseif ($hex == '65') {
				$delay = ord($data[$offset + 1]) + ord($data[$offset + 2]) * 256;
				$sum += $delay;
				$offset += 3;
			} elseif ($hex == '66' || $hex == '68') {
				$plen = ord($data[$offset + 1]) + ord($data[$offset + 2]) * 256;
				$offset += 3 + $plen;
			} elseif ($hex == '69') {
				$offset += 5;
			} else {
				$end = true;
			}

			if ($offset >= $movie_size) {
				$end = true;
			}
		}

		return $sum;
	}

	private function select(string $query)
	{
		mysqli_query($this->mysqlConnection, 'SET names utf8');
		$results = mysqli_query($this->mysqlConnection, $query);

		if (!is_object($results)) {
			return $results;
		}

		if (mysqli_errno($this->mysqlConnection)) {
			echo mysqli_error($this->mysqlConnection);
		}

		$return = [];

		while ($row = mysqli_fetch_assoc($results)) {
			$return[] = $row;
		}

		return $return;
	}
}

if (!is_file(__DIR__ . '/config.php')) {
	die('missing config.php file');
}

include __DIR__ . '/config.php';

if (!isset($config) || !isset($config['sqlHost']) || !isset($config['ftpHost'])) {
	die('invalid config.php file');
}

(new Cam())->execute($config);
