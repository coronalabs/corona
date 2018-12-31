def current_version
  File.read('Source/version.h').match(/IOS_SIM_VERSION "([\d\.]+)"/)[1]
end

desc "Create a Release build"
task :build do
  sh "xcodebuild clean -project ios-sim.xcodeproj"
  sh "xcodebuild -project ios-sim.xcodeproj -configuration Release SYMROOT=build"
end

desc "Install a Release build"
task :install => :build do
  if prefix = ENV['prefix']
    bin_dir = prefix
    if !(prefix =~ /bin$/)
      bin_dir = File.join(prefix, 'bin')
    end
    mkdir_p bin_dir
    cp 'build/Release/ios-sim', bin_dir
  elsif npm_prefix = ENV['npm_config_prefix']
    bin_dir = npm_prefix
    if !(npm_prefix =~ /bin$/)
      bin_dir = File.join(npm_prefix, 'bin')
    end
    mkdir_p bin_dir
    cp 'build/Release/ios-sim', bin_dir
  else
    puts "[!] Specify a directory as the install prefix with the `prefix' env variable"
    exit 1
  end
end

desc "Clean"
task :clean do
  rm_rf 'build'
end

desc "Update README.md from README.template.md"
task :readme do
  template = File.read('README.template.md')
  rendered = template.gsub('{{VERSION}}', current_version)
  File.open('README.md', 'w') { |f| f << rendered }
end

namespace :version do
  desc "Print the current version"
  task :current do
    puts current_version
  end

  desc "Bump the version and update the README.md"
  task :bump do
    if v = ENV['v']
      File.open('Source/version.h', 'w') do |f|
        f << %{#define IOS_SIM_VERSION "#{v}"}
      end
      Rake::Task[:readme].invoke
      sh "git commit Source/version.h README.md -m 'Release #{v}'"
      puts "Bumped version to #{current_version}"
    else
      puts "[!] Specify the version with the `v' env variable"
    end
  end
end

desc "Print the current version"
task :version => 'version:current'

desc "Release a new version"
task :release => 'version:bump' do
  sh "git push"
  sh "git push --tags"
end
