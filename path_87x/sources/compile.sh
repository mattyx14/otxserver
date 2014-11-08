######################
#Update Configuration#
#########################
#Do Not Change this info#
#########################
LuajitVer=-2.0.0-beta8
boostVer=.1.47.0
boostVer_=_1_47_0
#######################
#/Update Configuration#
#######################

if [[ $EUID -ne 0 ]]; then
	echo 'You are not root, type "su" to log in to root and run this script again. Press [ENTER] to end this script.'
	read end
	exit 1
else

################
#Chose Your OS #
################

	echo 'Chose your Operating System. {Supported OS: Debian, Ubuntu, Fedora, CentOS} '
	read ans1 
	if [ $ans1 = "Fedora" ] || [ $ans1 = "CentOS" ]; then

##########################
#TFS Dependencies & Tools#
##################################################
#This is all the dependencies needed to compile. #
##################################################

		echo -n "[$ans1]Do you want to install all Dependencies and Tools needed?[y/n] "
		read ans1_1
		if [ $ans1_1 = 'y' ]; then
			yum install mysql++-devel lua-devel boost-devel gmp-devel lua-sql-mysql lua-sql-sqlite libsqlite3x-devel zlib-devel libxml2-devel libxml++-devel kernel-devel mysql-connector-c++ mysql-connector-c++-devel && yum groupinstall "Development Tools"
		fi
		echo 'Continuing...'

	else	if [ $ans1 = "Debian" ] || [ $ans1 = "Ubuntu" ]; then
			echo -n "[$ans1]Do you want to install all Dependencies and Tools needed?[y/n] "
			read ans1_1
			if [ $ans1_1 = 'y' ]; then
				sudo apt-get install build-essential libxml2-dev libxml++2.6-dev liblua5.1-0-dev libboost-all-dev libmysql++-dev libgmp3-dev liblua5.1-sql-mysql-dev liblua5.1-sql-sqlite-dev libsqlite3-dev zlib1g-dev dh-autoreconf
				echo 'Libraries and Build Tools... Installed'
			else
				echo 'Continuing...'
			fi
		else
			echo ['Error: Can not Install Dependencies, this may be caused due to mistyping the OS name(OS name is case-sensitive) or that the OS is not supported.']
		fi
	fi
#################
#Compiling Boost#
##################################
# Credits to: Fallen & Cykotitan #
##################################

		echo -n 'Do you want to compile the latest boost?[y/n] '
		read ans2
		if [ $ans2 = 'y' ]; then
			wget http://voxel.dl.sourceforge.net/project/boost/boost/$boostVer/boost$boostVer_.tar.gz

			mv download boost.tar.gz

			tar xzf boost.tar.gz

			cd boost_1_47_0/

			chmod +x bootstrap.sh

			./bootstrap.sh

			./bjam variant=release link=static threading=multi

			echo "Boost$boostVer_ has been installed."

			cd ..
		else
			echo 'Continuing...'
		fi

#################
#Building LuaJIT#
##############################################################
#This will compile LuaJIT(Lua Just-In-Time) for your server. #
##############################################################

		echo -n 'Do you want to compile and install LuaJIT?[y/n] '
		read ans3
		if [ $ans3 = 'y' ]; then
			if [ -f "/usr/local/bin/luajit$LuajitVer" ]; then
				echo 'LuaJIT has already been installed.'
			else
				wget http://luajit.org/download/LuaJIT$LuajitVer.tar.gz

				tar zxf LuaJIT-2.0.0-beta8.tar.gz

				cd LuaJIT-2.0.0-beta8

				make && make install

				ln -sf luajit-2.0.0-beta8 /usr/local/bin/luajit

				cd ..
	
			fi
		fi
		echo 'Continuing...'

###############
#Compiling TFS#
#################################################
#This will compile TFS, it has default settings.#
#If you wish to specify more options, go follow ################
# the compiling instructions in DOC/README.     # -D__LUAJIT-_ #
################################################################

		if [ -f "autogen.sh" ]; then
			./autogen.sh
		else
			echo 'No autogen.sh found, moving on.'
		fi

		echo -n 'Do you want to specify additional flags?[y/n] '
		read ans4
		if [ $ans4 = 'y' ]; then
			if [ -f "configure.ac" ]; then	
				echo -n 'You are about to make changes to "configure.ac", when you are done editing press [CTRL]+X to exit and save, press [ENTER] to start editing. '
				read cntn1	
				nano configure.ac
				echo -n 'Do you want to enable additional features? '
				read cntn2
				if [ $cntn2 = 'yes' ]; then
					echo "Enable any feature you want, Full list at:[ http://otland.net/content/compiling-forgotten-server-debian-gnu-linux-16/ ] simply write it and press [ENTER]: "
					read addons
					./configure --enable-mysql $addons
				else
					./configure --enable-mysql
				fi
			else
				echo 'No configure.ac found, moving on.'
			fi
		else
			if [ -f "configure.ac" ]; then
				./configure --enable-mysql
			else
				echo 'No configure.ac found, moving on.'
			fi
		fi

		if [ -f "build.sh" ]; then
			./build.sh
		else
			mkdir obj
			make
		fi

		echo -n 'Do you wish to clean up the server folder?[y/n] '
		read ans5
		if [ $ans5 = 'y' ]; then
			mkdir src && mkdir objs

			mv *.cpp *.h src/ && mv *.o objs/
			#There are a few more files that are needed to be moved.
		fi
		echo -n 'Done, press [ENTER].'
		read finish

		exit 0
fi
####################################
#Reference Used in this compilation#
#######################################################################
# Compiling The Forgotten Server in Debian GNU/Linux By: Don Daniello ############         
# URL: http://otland.net/content/compiling-forgotten-server-debian-gnu-linux-16/ #
##################################################################################
