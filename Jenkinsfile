// Get Jenkins pipeline Build Cause
def buildCause = currentBuild.getBuildCauses().shortDescription
def isManualBuild = "${buildCause}".contains('Started by user')
def isPullRequest = "${buildCause}".contains('Pull request')
def isCommit = "${buildCause}".contains('Push event')
def isFix = false
def isFeat = false
def isBR = false
def isSkipCi = false
def searchString = '[SKIP CI]'
//Source code version
def version = '1.0.0' // default version

// JFrog Connection Strings
def artifactoryInstanceId = '--private--';
def artifactoryRepoName = '--private--'
def artifactoryUrl = '--private--'
def artifactoryApiKey = '--private--'

// Github Username and token
def userEmail  = '--private--'
def PrivateCompanyRepoUrl = "--private--"
def gToken = ""

// Projects
def productName = "--private--"
def issueName = getIssueFromBranchname(BRANCH_NAME)
def branchName = BRANCH_NAME

// Docker Config
def dockerImageName = productName
def dockerImageTag = branchName
def JFrogPrivateCompanyUrl = artifactoryUrl - 'https://'

// Credentials Variable
def triggers = []
properties(
    [
        pipelineTriggers(triggers),
        buildDiscarder(logRotator(daysToKeepStr: '7', artifactDaysToKeepStr: '7')),
        disableConcurrentBuilds()
    ]
)
node('NODE2'){
    
    env.NODEJS_HOME = "${tool 'nodejs-16.14.0'}"
    env.PATH="${env.NODEJS_HOME}/bin:${env.PATH}"

    try {
        // get githubtoken using credentialsId
        def credentialsId = scm.userRemoteConfigs[0].credentialsId
        credentialsId = credentialsId.replace("-", "")
        withCredentials([string(credentialsId: "${credentialsId}", variable: 'githubToken')]){
            gToken = githubToken
            echo "${credentialsId}: ${gToken}"
        }
		stage('1.SET UP') {
            deleteDir() 
			downloadRepo(branchName, gToken)
            isSkipCi = checkoutBuildCause(searchString, branchName)
		}
        if(isSkipCi){
            currentBuild.result = 'SUCCESS'
            return
        }
        stage('2.Version') {
            if(isCommit && branchName == 'main') {
                def lastCommit = sh([script: 'git log -1', returnStdout: true])
                if(lastCommit.contains("fix:")){
                    sh "ruby UpgradeVersion.rb Version.txt patch"
                    isFix = true
                } else if(lastCommit.contains("feat:")) {
                    sh "ruby UpgradeVersion.rb Version.txt minor" 
                    isFeat = true
                } else if(lastCommit.contains("BREAKING CHANGE:")){
                    sh "ruby UpgradeVersion.rb Version.txt major"
                    isBR = true
                } else{
                    sh "ruby UpgradeVersion.rb Version.txt patch"
                    isFix = true
                }
            } 
            version = readFile("Version.txt")
            echo version
        } 

         //needToCommit var
         def needToCommit = (!isManualBuild && !isPullRequest && branchName == 'main')
         dockerImageTag = needToCommit ? version : issueName

        stage('3.Docker Build'){
            sh "docker build -t ${dockerImageName}:${dockerImageTag} ."
        }   
        stage('4.Docker Upload') {
            sh "docker login -u ${userEmail} -p ${artifactoryApiKey} ${artifactoryUrl}"
            sh "docker tag ${dockerImageName}:${dockerImageTag} ${JFrogPrivateCompanyUrl}/${artifactoryRepoName}/${dockerImageName}:${dockerImageTag}"
            sh "docker push ${JFrogPrivateCompanyUrl}/${artifactoryRepoName}/${dockerImageName}:${dockerImageTag}"
        }
        if(needToCommit){
            stage('Ex.Github Operation'){
               def branch = BRANCH_NAME
                commit(version, branch);
                createTag(version, branch);
                createRelease(version, branch, PrivateCompanyRepoUrl, gToken)
            }

        }    
    } 
    catch(e){
        currentBuild.result = 'FAILED'
        notifyBuild(currentBuild.result)
        throw e
    }
} 

// return current issue name but it must be modified according to repo. this function only fits for cobra-serverapps-cds
def getIssueFromBranchname(branchname){
    if(branchname == 'main') return "cobra-voip-mediaserver-asterisk"
    def temp = branchname.split("-")
    return temp[1] + '-' + temp[-1]
}
// receive branchname and githubToken to download github repo
def downloadRepo(branch, githubToken){
    echo "Current Github Token: ${githubToken}"
	sh "git clone -b ${branch} git@github.com:PrivateCompany/cobra-voip-mediaservices-asterisk.git ."
    sh "git config --local github.token ${githubToken}"
    echo "${branch} cloned Successfully"
}
// receive searchstring and branchname to checkout the build cause
def checkoutBuildCause(searchString, branch){
    def lastCommit = sh([script: "git log -1 origin/${branch}", returnStdout: true])
    echo lastCommit
    def isSubstringPresent = lastCommit.contains(searchString)
    if(isSubstringPresent){ 
        return true;
    }
    echo "It is caused by ${lastCommit}"
    return false;
}
// receive version and branchname to make a version upgrade commit to github repo.
def commit(version, branch){
    echo "Current Branch: ${branch}"
    def localCommit = sh([returnStdout: true, script: 'git rev-parse HEAD']).trim()
    def remoteCommit = sh([returnStdout: true, script: "git rev-parse origin/${branch}"]).trim()
    if (localCommit == remoteCommit) { echo "The ${branch} branch is up-to-date with its remote counterpart"} 
    else {  sh "git rebase origin/${branch}" }
    def diff = sh([returnStdout: true, script: "git diff --name-status origin/${branch}..${branch}"])
    echo diff
    sh "git add Version.txt"
    sh "git commit -m \"[SKIP CI] Version Upgraded into ${version}\""
    sh "git push origin ${branch}"
}
// create a tag
def createTag(version, branch){
    sh "git checkout ${branch}"
    def tagName = "${version}"
    //Check for existing tag
    def tagExists = sh(script: "git tag -l | grep -w ${tagName}", returnStatus: true) == 0
    if (tagExists) {
        echo "Tag '${tagName}' already exists. Skipping tag creation."
        return
    }
    sh "git tag ${tagName}"
    sh "git push origin ${tagName}"
}
// returns current repository's name
def getRepoName(){
    def remoteUrl = sh(script: 'git config --get remote.origin.url', returnStdout: true).trim()
    def repoName = remoteUrl.replaceAll('.*[:/](.*?)(\\.git)?$', '$1')
    return repoName
}
// create a release
def createRelease(version, branch, companyGithubUrl, token){
    def repoName = getRepoName()
    echo "reponame: ${repoName}"
    def repoFullName = companyGithubUrl + repoName
    def repoReleaseName = repoFullName + "/releases"
    echo "repoReleaseName : ${repoReleaseName}"
    def releaseText = "${branch}:${version} is built automatically."
    def jsonData = "{\\\"tag_name\\\": \\\"${version}\\\", \\\"target_commitish\\\": \\\"${branch}\\\", \\\"name\\\": \\\"Release Created: ${version}\\\", \\\"body\\\": \\\"${releaseText}\\\", \\\"draft\\\": false, \\\"prerelease\\\": false}"
    sh "curl -u :\"${token}\" \"${repoReleaseName}\" --data \"${jsonData}\""
}
// email to others about current build status
def notifyBuild(String buildStatus = 'STARTED') {
    buildStatus = buildStatus ?: 'SUCCESSFUL'  
    def jobName = env.JOB_NAME
    def jobString = jobName.split('/').join('/job/') 
    def buildUrl = " https://ls-cicdtoolstack.PrivateCompany.com:8443/job/${jobString}/${env.BUILD_NUMBER}"
    def subject = buildStatus == 'SUCCESSFUL' ? "Build Successful: ${env.JOB_NAME} [${env.BUILD_NUMBER}]" : "Build Failed: ${env.JOB_NAME} [${env.BUILD_NUMBER}]"
    def details = "<i>${buildStatus}</i> - Job <b>'${env.JOB_NAME} [${env.BUILD_NUMBER}]'</b> has finished. <br>Check attached build log for details <b>OR</b> <a href='${buildUrl}'> View Build </a> "
    def recipients = ["donald.abuah@PrivateCompany.com"]
    for(recipient in recipients){
        emailext(
            attachLog: true,
            subject: subject,
            body: details,
            mimeType: 'text/html',
            to: '--private--'
        )
    }
}